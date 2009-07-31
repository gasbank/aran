#pragma once

class ArnSkeletonDx9
{
public:
										~ArnSkeletonDx9(void);
	static ArnSkeleton*					createFrom(const NodeBase* nodeBase);
	const SkeletonData&					getSkeletonData() const { return m_data; }

private:
										ArnSkeletonDx9(void);
	void								buildFrom(const NodeSkeleton1* ns);
	SkeletonData						m_data;
};
