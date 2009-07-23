#ifndef ARNXMLSTRING_H
#define ARNXMLSTRING_H

class ArnXmlString : public Singleton<ArnXmlString>
{
public:
	ArnXmlString();
	~ArnXmlString();

	XMLCh* TAG_object;
	XMLCh* TAG_scene;
	XMLCh* TAG_transform;
	XMLCh* TAG_scaling;
	XMLCh* TAG_rotation;
	XMLCh* TAG_translation;
	XMLCh* TAG_chunk;
	XMLCh* TAG_template;
	XMLCh* TAG_int;
	XMLCh* TAG_int4;
	XMLCh* TAG_float;
	XMLCh* TAG_float2;
	XMLCh* TAG_float3;
	XMLCh* TAG_arraydata;
	XMLCh* TAG_data;
	XMLCh* TAG_skeleton;
	XMLCh* TAG_bone;
	XMLCh* TAG_head;
	XMLCh* TAG_tail;
	XMLCh* TAG_roll;
	XMLCh* TAG_mesh;
	XMLCh* TAG_vertex;
	XMLCh* TAG_face;
	XMLCh* TAG_uv;
	XMLCh* TAG_vertexgroup;
	XMLCh* TAG_material;
	XMLCh* TAG_light;
	XMLCh* TAG_camera;
	XMLCh* TAG_diffuse;
	XMLCh* TAG_ambient;
	XMLCh* TAG_specular;
	XMLCh* TAG_emissive;
	XMLCh* TAG_power;
	XMLCh* TAG_texture;
	XMLCh* TAG_facegroup;
	XMLCh* TAG_field;
	XMLCh* TAG_ipo;
	XMLCh* TAG_curve;
	XMLCh* TAG_controlpoint;
	XMLCh* TAG_action;
	XMLCh* TAG_actionstrip;
	XMLCh* TAG_objectipomap;

	XMLCh* ATTR_name;
	XMLCh* ATTR_type;
	XMLCh* ATTR_place;
	XMLCh* ATTR_unit;
	XMLCh* ATTR_rtclass;
	XMLCh* ATTR_startoffset;
	XMLCh* ATTR_endoffset;
	XMLCh* ATTR_farclip;
	XMLCh* ATTR_nearclip;
	XMLCh* ATTR_fovdeg;
	XMLCh* ATTR_value;
	XMLCh* ATTR_path;
	XMLCh* ATTR_mtrl;
	XMLCh* ATTR_usage;
	XMLCh* ATTR_x;
	XMLCh* ATTR_y;
	XMLCh* ATTR_z;
	XMLCh* ATTR_w;
	XMLCh* ATTR_r;
	XMLCh* ATTR_g;
	XMLCh* ATTR_b;
	XMLCh* ATTR_a;
	XMLCh* ATTR_ipo;
	XMLCh* ATTR_obj;
	XMLCh* ATTR_shadeless;

	// Possible values for 'object rtclass' attribute
	XMLCh* VAL_ArnSceneGraph;
	XMLCh* VAL_ArnMesh;
	XMLCh* VAL_ArnLight;
	XMLCh* VAL_ArnCamera;
	XMLCh* VAL_ArnMaterial;
	XMLCh* VAL_ArnSkeleton;
	XMLCh* VAL_ArnIpo;
	XMLCh* VAL_ArnAction;
	XMLCh* VAL_ArnBone;

	// Possible values for 'chunk place' attribute
	XMLCh* VAL_xml;
	XMLCh* VAL_bin;

	// Possible values for 'transform type' attribute
	XMLCh* VAL_srt; // Scaling, Rotation, Translation
	XMLCh* VAL_matrix;

	// Possible values for 'rotation type' attribute
	XMLCh* VAL_euler; // Euler angle rotation
	XMLCh* VAL_quat; // Quaternion rotation

	// Possible values for 'rotation unit' attribute (Only useful when rotation type is euler.
	XMLCh* VAL_deg;
	XMLCh* VAL_rad;

	// Possible values for 'light type' attribute
	XMLCh* VAL_point;
	XMLCh* VAL_spot;
	XMLCh* VAL_directional;

	// Possible values for 'material texture type' attribute
	XMLCh* VAL_image;

	// Possible values for 'camera type' attribute
	XMLCh* VAL_persp;
	XMLCh* VAL_ortho;
};

inline ArnXmlString& GetArnXmlString() { return ArnXmlString::getSingleton(); }

class ScopedString
{
public:
	ScopedString(const XMLCh* x)
	: m_str(XMLString::transcode(x))
	{
		// Note: Empty string allowed.
		//assert(strlen(m_str));
	}
	~ScopedString()
	{
		XMLString::release(&m_str);
	}
	const char* c_str() const { return m_str; }
private:
	char* m_str;
};

#endif // ARNXMLSTRING_H
