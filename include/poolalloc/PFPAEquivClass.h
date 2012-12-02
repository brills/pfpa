#ifndef __PFPA_EQUIV_CLASS__H_
#define __PFPA_EQUIV_CLASS__H_
#include <vector>
#include <map>

namespace llvm {
struct PFPAEquivClass{

	//(offset, size)
	typedef std::pair<unsigned, unsigned> ProfileDataPairTy;
	//(offset, size) -> freq
	typedef std::map<ProfileDataPairTy, unsigned> FreqMapTy;
	//DSID -> ((offset, size) -> freq)
	typedef std::map<int, FreqMapTy> DSIDToProfileDataMapTy;
	const Type *DSType;
	std::set<int> DSIDInClass;
	DSIDToProfileDataMapTy DSIDToProfileDataMap;
	std::vector<GetElementPtrInst *> RelatedGEP;
	std::vector<CallInst *> PoolInitCalls;
	std::vector<CallInst *> PoolAllocCalls;
	int RefCount;
	
};

}

#endif

