#pragma once

class Remap
{
public:
	Remap() {
		oldCount_ = -1;
	}
	void SetOldCount(int count) {
		oldCount_ = count;
		map8_.clear();
		map8_.resize(count*8, -1);
		base_ = &map8_[0];
		assert(*base_ == -1);
	}
	void MapOldToNew(int old, int nu) {
		assert(old < oldCount_ && old >= 0);
		int * p = base_ + old*8;
		for (int i = 0; i < 8; ++i) {
			if (p[i] == nu) {
				return;
			}
			if (p[i] == -1) {
				p[i] = nu;
				return;
			}
		}
		// TODO: code should not be reach here!!
		//
		assert( !"A single vertex is split to more than 8 vertices!\n" );
	}
	template<typename T>
	void ForeachOld(int old, T & func) {
		assert(old < oldCount_ && old >= 0);
		int * ptr = base_ + old*8;
		for (int i = 0; i < 8; ++i) {
			if (*ptr != -1) {
				func(*ptr);
			}
		}
	}
	int GetNew(int old, int i) {
		if (oldCount_ == -1) return -1;
		assert(old < oldCount_ && old >= 0);
		int * ptr = base_ + old*8;
		if (i < 0 || i >= 8) return -1;
		return ptr[i];
	}
	int * base_;
	std::vector<int> map8_;
	int oldCount_;
};
