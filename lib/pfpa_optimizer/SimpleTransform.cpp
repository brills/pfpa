#include <iostream>
#include <ostream>
#include <string>
#include <map>
#include <set>

#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GetElementPtrTypeIterator.h"
#include "llvm/Constants.h"
#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "poolalloc/PoolAllocate_pfpa_optimizer.h"
#include "poolalloc/PFPAEquivClass.h"

using namespace std;
using namespace llvm;

namespace {
  class SimpleTransform : public ModulePass {
    std::vector<GetElementPtrInst*> geps;    
 
    TargetData* TD;
    PoolAllocate* PA;
    set<PFPAEquivClass*>* EC;

    Type* Int8Type;
    Type* Int32Type;
    Type* VoidType;
    Type* VoidPtrType;

    map<PFPAEquivClass*, map<unsigned,bool> > hotFields;

    map<PFPAEquivClass*, StructType*> mapToHotType;

    map<PFPAEquivClass*, map<unsigned,unsigned> > mapToHotOffset;

    public:

    static char ID;

    SimpleTransform() : ModulePass(ID){}

    virtual bool runOnModule(Module& M){
      LLVMContext & Context = M.getContext();
      Int8Type = IntegerType::getInt8Ty(Context);
      Int32Type = IntegerType::getInt32Ty(Context);
      VoidType = IntegerType::getVoidTy(Context);
      VoidPtrType = PointerType::getUnqual(Int8Type);
      errs() << "Processing...!\n";

      TD = &getAnalysis<TargetData>();
      PA = &getAnalysis<PoolAllocate>();
      EC = &PA->getPFPAEquivClassSet();

      Type* PoolDescType = PA->getPoolType(&Context); 
      Type* PoolDescPtrTy = PointerType::getUnqual(PoolDescType);

      Constant* poolInitOpt = M.getOrInsertFunction("poolinit_opt", VoidType,
                                  PoolDescPtrTy, Int32Type, Int32Type,
                                  Int32Type, Int32Type, Int32Type, NULL);

      Constant* poolAllocOpt = M.getOrInsertFunction("poolalloc_opt", VoidPtrType,
                                  PoolDescPtrTy, Int32Type, NULL);

      for(set<PFPAEquivClass*>::iterator i = EC->begin(), e = EC->end(); i!=e; ++i){
        PFPAEquivClass* ec = *i;
        StructType* type = const_cast<StructType*>((const StructType*)ec->DSType);

        bool isTransformed = processEC(ec);

        if(!isTransformed)
          continue;        

        for(unsigned i=0;i<ec->PoolInitCalls.size(); i++){
          CallInst* ci = ec->PoolInitCalls[i];

          unsigned hotSize = TD->getStructLayout(mapToHotType[ec])->getSizeInBytes();
          unsigned coldSize = TD->getStructLayout(type)->getSizeInBytes();

          Value* hotSizeValue = ConstantInt::get(Int32Type, hotSize);
          Value* coldSizeValue = ConstantInt::get(Int32Type, coldSize);
          Value* Opts[6] = {ci->getArgOperand(0), ci->getArgOperand(1), ci->getArgOperand(2),
                                  ci->getArgOperand(3), coldSizeValue, hotSizeValue}; 
          CallInst* newInst = CallInst::Create(poolInitOpt, Opts);
          ReplaceInstWithInst(ci, newInst); 
        }

        for(unsigned i=0;i<ec->PoolAllocCalls.size(); i++){
          CallInst* ca = ec->PoolAllocCalls[i];
          ca->setCalledFunction(poolAllocOpt);
        }

        vector<GetElementPtrInst *> geps = ec->RelatedGEP;

        // Step through related GetElementPtrInst's and fix references to the hot fields
        for(unsigned i=0; i<geps.size(); i++){
          unsigned offset = getOffset(geps[i]);
          unsigned newOffset;
          StructType* newType;
          if(!hotFields[ec][offset]){
            CastInst* ci = CastInst::CreatePointerCast(geps[i]->getPointerOperand(), PointerType::getUnqual(geps[i]->getPointerOperandType()));
            ci->insertBefore(geps[i]);
            LoadInst* li = new LoadInst(ci, "coldAccess", geps[i]);
            //errs() << *li->getType() << "\n";
            geps[i]->setOperand(0,li);
          } else {
            newOffset = mapToHotOffset[ec][offset];
            newType = mapToHotType[ec];
          
            Value* v = geps[i]->getOperand(0);
            CastInst* ci = CastInst::CreatePointerCast(v, PointerType::getUnqual(newType));
            ci->insertBefore(geps[i]);
            geps[i]->setOperand(0, ci);
            geps[i]->setOperand(2, ConstantInt::get(Int32Type, newOffset));
          }
        }
      }
      return true;
    }

 /**
   * Check profiled data for fields to optimize and create new types if necessary
   *
   * Returns true if the type has been transformed
   */
    bool processEC(PFPAEquivClass* ec){
      StructType* type = const_cast<StructType*>((const StructType*)ec->DSType);
      errs() << "***Analyzing " << *type << "\n";

      const StructLayout* layout = TD->getStructLayout(type);

      bool isTransformed = false;

      map<unsigned, unsigned> offsetToCounts;

      // If equivalence class contains more than 1 DSID, combine the statistics and analyze based on that
      unsigned total = 0;
      for(set<int>::iterator di = ec->DSIDInClass.begin(), de = ec->DSIDInClass.end(); di!=de; ++di){
        int DSID = *di;
        
        PFPAEquivClass::FreqMapTy stats = ec->DSIDToProfileDataMap[DSID];
        for(PFPAEquivClass::FreqMapTy::iterator fr = stats.begin(), fre = stats.end(); 
                        fr!=fre; ++fr){
          unsigned offset = (*fr).first.first;
          unsigned count = (*fr).second;
          errs() << "OS:" << offset << "->" << count << "\n";
          total += count;

          offsetToCounts[offset] += count;
        }
      }

      // Pick the hot fields.
      //
      // Simple heuristic: If field is accessed more than 20% of all accesses, it is hot.
      map<unsigned,bool> hotMap;
      for(unsigned i=0; i<((const StructType*)type)->getNumElements(); i++){
        unsigned offset = layout->getElementOffset(i);
        double k = (double)offsetToCounts[offset]/(double)total;
 
        errs() << "\t" << offset << ": " << offsetToCounts[offset] << "/" << total << "\n";
        if(k>.2){
          hotMap[offset] = true;
        } else {
          isTransformed = true;
        }
      }

      if(isTransformed){
        createNewTypes(ec, hotMap);
        errs() << "\t New Type Created: " << *mapToHotType[ec] << "\n";
      }

      return isTransformed;
    }

    unsigned getOffset(GetElementPtrInst* gep){
      //Calculate offset from GEP
      unsigned offset = 0;
      if(StructType *STy = dyn_cast<StructType>(((PointerType*)gep->getPointerOperandType())->getElementType())){
        const ConstantInt* CUI = cast<ConstantInt>(gep->getOperand(2));
        int FieldNo = CUI->getSExtValue();

        offset = (unsigned)TD->getStructLayout(STy)->getElementOffset(FieldNo);
      }

      return offset;
    }

    
   /**
    * Create a new type for the equivalence class based on the passed hot map.
    *
    * It also populates the ec to type maps and the old offset to new offset maps.
    *
    **/
    void createNewTypes(PFPAEquivClass* ec, map<unsigned,bool> hotMap){
      StructType* type = const_cast<StructType*>((const StructType*)ec->DSType);
      const StructLayout* sl = TD->getStructLayout(type);

      StructType* hotType;

      vector<Type*> hot;

      hot.push_back(VoidPtrType);

      vector<pair<unsigned,unsigned> > offsetToIdx;

      unsigned idx = 0;
      for(StructType::element_iterator ei = type->element_begin(), ee = type->element_end();
			ei != ee; ++ei){
        Type* t = *ei;
        unsigned offset = sl->getElementOffset(idx++);

        if(hotMap[offset]){
          hot.push_back(t);
          hotFields[ec][offset] = true;
          mapToHotOffset[ec][offset] = hot.size()-1;
        }
      }

      hotType = StructType::create(ArrayRef<Type*>(hot), (type->getName() + ".hot").str());
      mapToHotType[ec] = hotType;
    }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<TargetData>();
      AU.addRequired<PoolAllocate>();
    } 
  };
}

char SimpleTransform::ID = 0;
static RegisterPass<SimpleTransform> X("st", "Simple Transform", true, true);
