// ExampleParser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static void Cleanup()
{
	ArnCleanupXmlParser();
	ArnCleanupImageLibrary();
	ArnCleanupPhysics();
}

int main(int argc, const char* argv[])
{
	if (argc != 2)
	{
		printf("Please provide the input XML file name as the first argument.\n");
		return -2;
	}
	const char *fileName = argv[1];

	if (ArnInitializeXmlParser() < 0)
	{
		Cleanup();
		return -1;
	}
	if (ArnInitializeImageLibrary() < 0)
	{
		Cleanup();
		return -2;
	}
	if (ArnInitializePhysics() < 0)
	{
		Cleanup();
		return -3;
	}
	ArnSceneGraph *sg = ArnSceneGraph::createFrom(fileName);
	if (!sg)
	{
		fprintf(stderr, " *** Scene graph file is not loaded correctly.\n", fileName);
		fprintf(stderr, "     Check your input XML scene file.\n");
		exit(-1);
	}
	sg->interconnect(sg);
	std::cout << "   Scene file " << fileName << " loaded successfully." << std::endl;

	printf(" ================ [ Scene Graph ] ================\n");
	sg->printNodeHierarchy(0);
	

	SimWorld *sw = SimWorld::createFrom(sg);

	printf(" ================ [ Rigid Bodies ] ================\n");
	foreach (const GeneralBodyPtr &b, sw->getGeneralBodyPtrSet())
	{
		printf ("Body: %s (%f)\n", b->getName(), b->getMass());
	}

	printf(" ================ [ Joints ] ================\n");
	foreach (const GeneralJointPtr &j, sw->getGeneralJointPtrSet())
	{
		printf ("Joint: %s (%s)\n", j->getName(), j->getJointTypeName());
		const ArnJointData &ajd = j->getJointData();
		printf ("   > Axis "); ajd.ax.printFormatString();
		printf ("   > Pivot "); ajd.pivot.printFormatString();
		foreach (const ArnJointData::ArnJointLimit& ajl, ajd.limits)
		{
			printf("   > %s [%f ~ %f]\n", ajl.type.c_str(), ajl.minimum, ajl.maximum);
		}
	}

	delete sw;
	delete sg;

	ArnCleanupXmlParser();
	ArnCleanupImageLibrary();
	ArnCleanupPhysics();

	return 0;
}

