#include "AranPCH.h"
#include "ArnXmlString.h"

IMPLEMENT_SINGLETON(ArnXmlString)

ArnXmlString::ArnXmlString()
{
#define TAG_INIT(x) TAG_##x = xercesc::XMLString::transcode(#x);
	TAG_INIT(object)
	TAG_INIT(scene)
	TAG_INIT(object)
	TAG_INIT(transform)
	TAG_INIT(scaling);
	TAG_INIT(rotation);
	TAG_INIT(translation);
	TAG_INIT(chunk)
	TAG_INIT(template)
	TAG_INIT(int)
	TAG_INIT(int4)
	TAG_INIT(float)
	TAG_INIT(float2)
	TAG_INIT(float3)
	TAG_INIT(arraydata)
	TAG_INIT(data)
	TAG_INIT(skeleton)
	TAG_INIT(bone)
	TAG_INIT(head)
	TAG_INIT(tail)
	TAG_INIT(roll)
	TAG_INIT(mesh)
	TAG_INIT(vertex)
	TAG_INIT(face)
	TAG_INIT(uv)
	TAG_INIT(vertexgroup)
	TAG_INIT(material)
	TAG_INIT(light)
	TAG_INIT(camera)
	TAG_INIT(diffuse)
	TAG_INIT(ambient)
	TAG_INIT(specular)
	TAG_INIT(emissive)
	TAG_INIT(power)
	TAG_INIT(texture)
	TAG_INIT(facegroup)
	TAG_INIT(field)
	TAG_INIT(ipo)
	TAG_INIT(curve);
	TAG_INIT(controlpoint);
	TAG_INIT(action);
	TAG_INIT(actionstrip);
	TAG_INIT(objectipomap);
#undef TAG_INIT

#define ATTR_INIT(x) ATTR_##x = xercesc::XMLString::transcode(#x);
	ATTR_INIT(name)
	ATTR_INIT(type)
	ATTR_INIT(place)
	ATTR_INIT(unit)
	ATTR_INIT(rtclass)
	ATTR_INIT(startoffset)
	ATTR_INIT(endoffset)
	ATTR_INIT(farclip)
	ATTR_INIT(nearclip)
	ATTR_INIT(fovdeg)
	ATTR_INIT(value)
	ATTR_INIT(path)
	ATTR_INIT(mtrl)
	ATTR_INIT(usage)
	ATTR_INIT(x)
	ATTR_INIT(y)
	ATTR_INIT(z)
	ATTR_INIT(w)
	ATTR_INIT(r)
	ATTR_INIT(g)
	ATTR_INIT(b)
	ATTR_INIT(a)
	ATTR_INIT(ipo)
	ATTR_INIT(obj)
	ATTR_INIT(shadeless);
#undef ATTR_INIT

#define VAL_INIT(x) VAL_##x = xercesc::XMLString::transcode(#x);
	VAL_INIT(ArnSceneGraph)
	VAL_INIT(ArnMesh)
	VAL_INIT(ArnLight)
	VAL_INIT(ArnCamera)
	VAL_INIT(ArnMaterial)
	VAL_INIT(ArnSkeleton)
	VAL_INIT(ArnIpo)
	VAL_INIT(ArnAction)
	VAL_INIT(ArnBone)

	VAL_INIT(xml)
	VAL_INIT(bin)

	VAL_INIT(srt);
	VAL_INIT(matrix);

	VAL_INIT(euler);
	VAL_INIT(quat);

	VAL_INIT(deg);
	VAL_INIT(rad);

	VAL_INIT(point);
	VAL_INIT(spot);
	VAL_INIT(directional);

	VAL_INIT(image);

	VAL_INIT(persp);
	VAL_INIT(ortho);
#undef VAL_INIT
}

ArnXmlString::~ArnXmlString()
{
#define TAG_RELEASE(x) xercesc::XMLString::release(&TAG_##x);
	TAG_RELEASE(object)
	TAG_RELEASE(scene)
	TAG_RELEASE(transform)
	TAG_RELEASE(scaling);
	TAG_RELEASE(rotation);
	TAG_RELEASE(translation);
	TAG_RELEASE(chunk)
	TAG_RELEASE(template)
	TAG_RELEASE(int)
	TAG_RELEASE(int4)
	TAG_RELEASE(float)
	TAG_RELEASE(float2)
	TAG_RELEASE(float3)
	TAG_RELEASE(arraydata)
	TAG_RELEASE(data)
	TAG_RELEASE(skeleton)
	TAG_RELEASE(bone)
	TAG_RELEASE(head)
	TAG_RELEASE(tail)
	TAG_RELEASE(roll)
	TAG_RELEASE(mesh)
	TAG_RELEASE(vertex)
	TAG_RELEASE(face)
	TAG_RELEASE(uv)
	TAG_RELEASE(vertexgroup)
	TAG_RELEASE(material)
	TAG_RELEASE(light)
	TAG_RELEASE(camera)
	TAG_RELEASE(diffuse)
	TAG_RELEASE(ambient)
	TAG_RELEASE(specular)
	TAG_RELEASE(emissive)
	TAG_RELEASE(power)
	TAG_RELEASE(texture)
	TAG_RELEASE(facegroup)
	TAG_RELEASE(field)
	TAG_RELEASE(ipo);
	TAG_RELEASE(curve);
	TAG_RELEASE(controlpoint);
	TAG_RELEASE(action);
	TAG_RELEASE(actionstrip);
	TAG_RELEASE(objectipomap);
#undef TAG_RELEASE

#define ATTR_RELEASE(x) xercesc::XMLString::release(&ATTR_##x);
	ATTR_RELEASE(name)
	ATTR_RELEASE(type)
	ATTR_RELEASE(place)
	ATTR_RELEASE(unit)
	ATTR_RELEASE(rtclass)
	ATTR_RELEASE(startoffset)
	ATTR_RELEASE(endoffset)
	ATTR_RELEASE(farclip)
	ATTR_RELEASE(nearclip)
	ATTR_RELEASE(fovdeg)
	ATTR_RELEASE(value)
	ATTR_RELEASE(path)
	ATTR_RELEASE(mtrl)
	ATTR_RELEASE(usage)
	ATTR_RELEASE(x)
	ATTR_RELEASE(y)
	ATTR_RELEASE(z)
	ATTR_RELEASE(w)
	ATTR_RELEASE(r)
	ATTR_RELEASE(g)
	ATTR_RELEASE(b)
	ATTR_RELEASE(a)
	ATTR_RELEASE(ipo)
	ATTR_RELEASE(obj)
	ATTR_RELEASE(shadeless);
#undef ATTR_RELEASE

#define VAL_RELEASE(x) xercesc::XMLString::release(&VAL_##x);
	VAL_RELEASE(ArnSceneGraph)
	VAL_RELEASE(ArnMesh)
	VAL_RELEASE(ArnLight)
	VAL_RELEASE(ArnCamera)
	VAL_RELEASE(ArnMaterial)
	VAL_RELEASE(ArnSkeleton)
	VAL_RELEASE(ArnIpo)
	VAL_RELEASE(ArnAction)
	VAL_RELEASE(ArnBone)

	VAL_RELEASE(xml)
	VAL_RELEASE(bin)

	VAL_RELEASE(srt);
	VAL_RELEASE(matrix);

	VAL_RELEASE(euler);
	VAL_RELEASE(quat);

	VAL_RELEASE(deg);
	VAL_RELEASE(rad);

	VAL_RELEASE(point);
	VAL_RELEASE(spot);
	VAL_RELEASE(directional);

	VAL_RELEASE(image);

	VAL_RELEASE(persp);
	VAL_RELEASE(ortho);
#undef VAL_RELEASE
}
