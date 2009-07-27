#ifndef ARNXMLSTRING_H
#define ARNXMLSTRING_H

typedef std::tr1::shared_ptr<XMLCh> XMLChPtr;

class XMLStringDeleter
{
public:
	void operator () (char*& d) const
	{
		xercesc::XMLString::release(&d);
	}
	void operator () (XMLCh*& d) const
	{
		xercesc::XMLString::release(&d);
	}
};

class ARAN_API ArnXmlString : public Singleton<ArnXmlString>
{
public:
	ArnXmlString();
	~ArnXmlString();

	XMLChPtr TAG_object;
	XMLChPtr TAG_scene;
	XMLChPtr TAG_transform;
	XMLChPtr TAG_scaling;
	XMLChPtr TAG_rotation;
	XMLChPtr TAG_translation;
	XMLChPtr TAG_chunk;
	XMLChPtr TAG_template;
	XMLChPtr TAG_int;
	XMLChPtr TAG_int4;
	XMLChPtr TAG_float;
	XMLChPtr TAG_float2;
	XMLChPtr TAG_float3;
	XMLChPtr TAG_arraydata;
	XMLChPtr TAG_data;
	XMLChPtr TAG_skeleton;
	XMLChPtr TAG_bone;
	XMLChPtr TAG_head;
	XMLChPtr TAG_tail;
	XMLChPtr TAG_roll;
	XMLChPtr TAG_mesh;
	XMLChPtr TAG_vertex;
	XMLChPtr TAG_face;
	XMLChPtr TAG_uv;
	XMLChPtr TAG_vertexgroup;
	XMLChPtr TAG_material;
	XMLChPtr TAG_light;
	XMLChPtr TAG_camera;
	XMLChPtr TAG_diffuse;
	XMLChPtr TAG_ambient;
	XMLChPtr TAG_specular;
	XMLChPtr TAG_emissive;
	XMLChPtr TAG_power;
	XMLChPtr TAG_texture;
	XMLChPtr TAG_facegroup;
	XMLChPtr TAG_field;
	XMLChPtr TAG_ipo;
	XMLChPtr TAG_curve;
	XMLChPtr TAG_controlpoint;
	XMLChPtr TAG_action;
	XMLChPtr TAG_actionstrip;
	XMLChPtr TAG_objectipomap;

	XMLChPtr ATTR_name;
	XMLChPtr ATTR_type;
	XMLChPtr ATTR_place;
	XMLChPtr ATTR_unit;
	XMLChPtr ATTR_rtclass;
	XMLChPtr ATTR_startoffset;
	XMLChPtr ATTR_endoffset;
	XMLChPtr ATTR_farclip;
	XMLChPtr ATTR_nearclip;
	XMLChPtr ATTR_fovdeg;
	XMLChPtr ATTR_value;
	XMLChPtr ATTR_path;
	XMLChPtr ATTR_mtrl;
	XMLChPtr ATTR_usage;
	XMLChPtr ATTR_x;
	XMLChPtr ATTR_y;
	XMLChPtr ATTR_z;
	XMLChPtr ATTR_w;
	XMLChPtr ATTR_r;
	XMLChPtr ATTR_g;
	XMLChPtr ATTR_b;
	XMLChPtr ATTR_a;
	XMLChPtr ATTR_ipo;
	XMLChPtr ATTR_obj;
	XMLChPtr ATTR_shadeless;

	// Possible values for 'object rtclass' attribute
	XMLChPtr VAL_ArnSceneGraph;
	XMLChPtr VAL_ArnMesh;
	XMLChPtr VAL_ArnLight;
	XMLChPtr VAL_ArnCamera;
	XMLChPtr VAL_ArnMaterial;
	XMLChPtr VAL_ArnSkeleton;
	XMLChPtr VAL_ArnIpo;
	XMLChPtr VAL_ArnAction;
	XMLChPtr VAL_ArnBone;

	// Possible values for 'chunk place' attribute
	XMLChPtr VAL_xml;
	XMLChPtr VAL_bin;

	// Possible values for 'transform type' attribute
	XMLChPtr VAL_srt; // Scaling, Rotation, Translation
	XMLChPtr VAL_matrix;

	// Possible values for 'rotation type' attribute
	XMLChPtr VAL_euler; // Euler angle rotation
	XMLChPtr VAL_quat; // Quaternion rotation

	// Possible values for 'rotation unit' attribute (Only useful when rotation type is euler.
	XMLChPtr VAL_deg;
	XMLChPtr VAL_rad;

	// Possible values for 'light type' attribute
	XMLChPtr VAL_point;
	XMLChPtr VAL_spot;
	XMLChPtr VAL_directional;

	// Possible values for 'material texture type' attribute
	XMLChPtr VAL_image;

	// Possible values for 'camera type' attribute
	XMLChPtr VAL_persp;
	XMLChPtr VAL_ortho;
};

inline ArnXmlString& GetArnXmlString() { return ArnXmlString::getSingleton(); }

class ScopedString
{
public:
	ScopedString(const XMLCh* x)
	: m_str(xercesc::XMLString::transcode(x), XMLStringDeleter())
	{
		// Note: Empty string allowed.
		//assert(strlen(m_str));
	}
	~ScopedString()
	{
	}
	const char* c_str() const { return m_str.get(); }
private:
	std::tr1::shared_ptr<char> m_str;
};

#endif // ARNXMLSTRING_H
