#include "kwxport.h"
#include "resource.h"

#include <d3dx9xof.h>
#include "rmxftmpl.h"
#include "rmxfguid.h"
#include <d3dx9mesh.h>

#include <dbghelp.h>
#include <hold.h>
#include <quat.h>
#include <IDxMaterial.h>

//#pragma optimize("", off)

#pragma comment(lib, "bmm.lib")
#pragma warning(disable: 4312) // cast int/pointer

static int const labelWidth = 110;
static int const rowHeight = 18;
static int const numCols = 1;

GMatrix globalMat;
typedef std::string string;

#define IDB_ADD 39001
#define IDB_UPDATE 39002
#define IDB_REMOVE 39003
#define IDC_ANIMATIONS 39004
#define IDC_FIRSTFRAME 39005
#define IDC_NUMFRAMES 39006
#define IDC_STRETCHTIME 39007
#define IDC_ANIMATIONNAME 39008
#define IDC_STATICFRAME 39009

#define ANIMATIONS_LIST_VIEW "allAnimations"

struct AnimationRange {
	std::string name;
	int first;
	int length;
	float timeStretch;
};

HMODULE dbgHelp;
BOOL (WINAPI *WriteDumpFunc)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, 
							 PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

float zMultiply = 1.0f;

void GlobalMirror(GMatrix &m)
{
	((float *)&m)[14] *= zMultiply;
	((float *)&m)[2] *= zMultiply;
	((float *)&m)[6] *= zMultiply;
	((float *)&m)[8] *= zMultiply;
	((float *)&m)[9] *= zMultiply;
}

bool CheckBad(float &f) {
	//  these tests weed out NaNs and the like
	if (f > 1.0 && f < 0.9) return true;
	if (!(f > 0.9) && !(f < 1.0)) return true;
	//  test for too large numbers (and inf)
	if (f > 1e30) return true;
	if (f < -1e30) return true;
	//  nuke denormals
	if (::fabsf(f) < 1e-30) f = 0;
	//  OK, the number is acceptable
	return false;
}

void GlobalMirror(Point3 &p)
{
	if (CheckBad(p.x) || CheckBad(p.y) || CheckBad(p.z)) {
		DebugPrint("Bad point in scene: %.3f,%.3f,%.3f\n", p.x, p.y, p.z);
		p.x = 1; p.y = 0; p.z = 0;
	}
	p.z *= zMultiply;
}

static DWORD MiniDumpHandler(LPEXCEPTION_POINTERS lpe)
{
	if (WriteDumpFunc != 0) {
		HANDLE file = ::CreateFile("c:\\kwxport.dmp", GENERIC_READ | GENERIC_WRITE, 
			0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (file != INVALID_HANDLE_VALUE) {
			MINIDUMP_EXCEPTION_INFORMATION x;
			x.ThreadId = ::GetCurrentThreadId();
			x.ExceptionPointers = lpe;
			x.ClientPointers = false;
			::OutputDebugString("Writing minidump to c:\\kwxport.dmp\n");
			DWORD procId = ::GetCurrentProcessId();
			HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, procId);
			BOOL b = (*WriteDumpFunc)(hProc, procId, file, 
				MiniDumpNormal, //MiniDumpWithIndirectlyReferencedMemory,
				&x, 0, 0);
			DWORD err = ::GetLastError();
			char buf[128];
			sprintf(buf, "File c:\\kwxport.dmp, result %d, error 0x%08lx\n", b, err);
			::OutputDebugString(buf);
			::FlushFileBuffers(file);
			::CloseHandle(file);
			::CloseHandle(hProc);
			::OutputDebugString("Done writing minidump to c:\\kwxport.dmp\n");
		}
		else {
			::OutputDebugString("Could not create minidump file at c:\\kwxport.dmp\n");
		}
	}
	else {
		::OutputDebugString("Minidump writing is disabled.\n");
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

template<typename T>
void GetAnimationRanges(std::string const & b, T & cont)
{
	AnimationRange ar;
	char const * x = b.c_str();
	char * e = (char *)x;
	long l = strtol(x, &e, 10);
	if (e != x) {
		x = e+1;
		for (long q = 0; q < l; ++q) {
			//  what a horrible parsing loop!
			e = (char *)strchr(x, ',');
			if (!e) goto done;
			std::string wt(x, e-x);
			ar.name = wt;
			x = e+1;
			e = (char *)strchr(x, ',');
			if (!e) goto done;
			wt = std::string(x, e);
			ar.first = strtol(wt.c_str(), (char **)&x, 10);
			x = e+1;
			e = (char *)strchr(x, ',');
			if (!e) goto done;
			wt = std::string(x, e);
			ar.length = strtol(wt.c_str(), (char **)&x, 10);
			x = e+1;
			e = (char *)strchr(x, ';');
			if (!e) goto done;
			wt = std::string(x, e);
			ar.timeStretch = (float)strtod(wt.c_str(), (char **)&x);
			x = e+1;
			cont.push_back(ar);
		}
done:
		;
	}
}

int WindowId(int i)
{
	if (i < 1000) return 40000 + i;
	return i;
}

static Point3 TransformPoint(GMatrix const & gm, Point3 const & p)
{
	float out[3];
	float const * v = &p.x;
	GRow const * m = gm.GetAddr();
	for (int i = 0; i < 3; ++i) {
		float o = m[3][i];
		for (int j = 0; j < 3; ++j) {
			o += m[j][i] * v[j];
		}
		out[i] = o;
	}
	return (Point3 &)out;
}

static Point3 TransformVector(GMatrix const & gm, Point3 const & p)
{
	float out[3];
	float const * v = &p.x;
	GRow const * m = gm.GetAddr();
	for (int i = 0; i < 3; ++i) {
		float o = 0;
		for (int j = 0; j < 3; ++j) {
			o += m[j][i] * v[j];
		}
		out[i] = o;
	}
	return (Point3 &)out;
}

static string wtocstr(wchar_t const * wstr)
{
	size_t l = wcslen(wstr);
	if (l > 1023) {
		l = 1023;
	}
	char buf[1024], *ptr = buf;
	while (l > 0) {
		*ptr = ((unsigned short)*wstr < 0xff) ? *wstr : '?';
		++ptr;
		++wstr;
		--l;
	}
	*ptr = 0;
	return string(buf);
}

#undef assert
#if !defined(NDEBUG)
#define assert(x) if (x); else assertion_failure(#x, __FILE__, __LINE__)
void assertion_failure(char const * expr, char const * file, int line)
{
	static std::string str;
	str = "Internal error in kW X-porter:\n\n";
	str += expr;
	str += "\n\n";
	str += file;
	str += ":";
	char buf[20];
	_snprintf(buf, 20, "%d", line);
	buf[19] = 0;
	str += buf;
	str += "\n\nStop the export attempt?";
	if (::MessageBox(0, str.c_str(), "Assertion Failure!", MB_YESNO) == IDYES) {
		throw "Assertion failure prevented export";
	}
}
#else
#define assert(x) (void)0
#endif

HINSTANCE hInstance;

static HRESULT hr;

#define CHECK(x) \
	hr = x; if( FAILED(hr) ) { ThrowHresult(#x,__FILE__,__LINE__,hr); }

static void ThrowHresult(char const * func, char const * file, int line, HRESULT h)
{
	static char buf[4096];
	char const * err = ::DXGetErrorString9(h);
	if (!err) {
		::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
			0, h, 0, (LPSTR)&err, 0, 0);
	}
	sprintf(buf, "Error saving .X file: 0x%08x (%s)\n%s:%d: %s\n", h, err ? err : "Unknown error", 
		file, line, func);
	buf[4095] = 0;
	::OutputDebugString(buf);
	throw buf;
}


BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
	switch ( nReason ) {
case DLL_PROCESS_ATTACH:
	hInstance = hDllHandle;
	break;
case DLL_PROCESS_DETACH:
	break;
case DLL_THREAD_ATTACH:
	break;
case DLL_THREAD_DETACH:
	break;
	}
	return TRUE;
}

ClassDesc2* GetIGameExporterDesc();

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return _T("Kilowatt X file exporter");
}

__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
case 0: return GetIGameExporterDesc();
default: return 0;
	}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}



#define KWXPORT_CLASS_ID	Class_ID(0x9387444A, 0xC8476283)

//corresponds to XML schema
TCHAR* mapSlotNames[] = {
	_T("Ambient"),
	_T("Diffuse"),
	_T("Specular"),
	_T("Glossiness"),
	_T("SpecularLevel"),
	_T("SelfIllumination"),
	_T("Opacity"),
	_T("Filter"),
	_T("Bump"),
	_T("Reflection"),
	_T("Refraction"),
	_T("Displacement"),
	_T("Unknown") };


	struct Settings {
		Settings();
		bool exportNormal_;
		bool exportTangent_;
		bool exportColor_;
		bool flipV_;
		bool flipZ_;
		int exportUv_;
		float scaleNumer_;
		float scaleDenom_;
		bool exportSkinning_;
		bool exportBinary_;
		bool exportMaterial_;
		bool fullPath_;
		bool renameTex_;
		std::string prefixTex_;
		bool ignoreUntextured_;
		bool exportShader_;
		bool exportAnimation_;
		bool exportComments_;
		int staticFrame_;
		int firstFrame_;
		int numFrames_;
		float timeStretch_;
		std::string animationName_;
		std::string allAnimations_;
	};
	//  Users of visitor may rely on the fact that the same string 
	//  constant will always be passed as the "name" parameter (the 
	//  third parameter).
	template<typename Visitor> bool Visit(Settings & s, Visitor & v)
	{
		return
			v("Geometry", s.exportNormal_, "normals", true, "Export Normals", 1) &&
			v("Geometry", s.exportTangent_, "tangents", false, "Export Tangents", 2) &&
			v("Geometry", s.exportColor_, "colors", true, "Export Colors", 3) &&
			v("Geometry", s.exportUv_, "uvChannels", 2, "Num UV Channels", 4) &&
			v("Geometry", s.flipV_, "flipV", false, "Flip V Channel", 9) &&
			v("Geometry", s.flipZ_, "flipZ", false, "Mirror Z Coordinate", 10) &&
			v("Geometry", s.scaleNumer_, "scaleNumer", 1.0f, "Scale Numerator", 5) &&
			v("Geometry", s.scaleDenom_, "scaleDenom", 100.0f, "Scale Denominator", 6) &&
			v("Geometry", s.exportSkinning_, "exportSkinning", false, "Export Skinning", 7) &&
			v("Geometry", s.exportBinary_, "exportBinary", false, "Export Binary", 8) &&
			v("Materials", s.exportMaterial_, "materials", true, "Export Materials", 20) &&
			v("Materials", s.fullPath_, "fullPath", false, "Full Texture Path", 21) &&
			v("Materials", s.renameTex_, "renameDds", true, "Rename to DDS", 22) &&
			v("Materials", s.prefixTex_, "prefixTex", std::string("textures/"), "Prefix", 23) &&
			v("Materials", s.ignoreUntextured_, "ignoreUntextured", false, "Ignore Untextured", 24) &&
			v("Materials", s.exportShader_, "exportShader", false, "Export Shader", 25) &&
			v("Animation", s.exportAnimation_, "exportAnimation", false, "Export Animation", 40) &&
			v("Animation", s.staticFrame_, "staticFrame", -1, "Static Frame", IDC_STATICFRAME) &&
			v("Animation", s.firstFrame_, "firstFrame", 0, "First Frame", IDC_FIRSTFRAME) &&
			v("Animation", s.numFrames_, "numFrames", 100, "Num. Frames", IDC_NUMFRAMES) &&
			v("Animation", s.timeStretch_, "timeStretch", 1.0f, "Stretch Time", IDC_STRETCHTIME) &&
			v("Animation", s.animationName_, "animationName", std::string("Idle"), "Animation Name", IDC_ANIMATIONNAME) &&
			v("Animation", s.allAnimations_, ANIMATIONS_LIST_VIEW, std::string(""), "Animations", IDC_ANIMATIONS) &&
			v("Misc", s.exportComments_, "comments", false, "Export Comments", 60) &&
			true;
	}
	struct SetDefaults {
		template<typename T>
		bool operator()(char const * category, T & b, char const * name, T value, char const * desc, int id) {
			b = value;
			return true;
		}
	};

	Settings::Settings()
	{
		Visit(*this, SetDefaults());
	}

	struct FatVert {
		int           origVert;
		Point3        pos;
		Point3        normal;
		Point3        tangent;
		Point3        binormal;
		Point2        uv[2];
		Point4        color;
	};


	class IGameExporter : public SceneExport, public Settings {
	public:

		std::vector<IGameNode *> animated_;
		IGameScene * igame_;
		Interface * core_;
		std::string name_;
		char exportTime_[100];
		TimeValue coreFrame_;

		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		BOOL SupportsOptions(int ext, DWORD options);
		int	DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
		int	DoExport2(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

		void WriteXFile(IGameScene * ig, TCHAR const * name);
		template<typename T> void ExportNode(IGameScene * ig, IGameNode * node, T * root);
		void WriteGeometry(IGameScene * ig, IGameNode * node, IGameObject * obj, ID3DXFileSaveData * frame);
		void WriteGeometry2(IGameScene * ig, IGameNode * node, IGameObject * obj, ID3DXFileSaveData * frame);
		static DWORD VertexAdd(std::vector<FatVert> & vec, std::vector<std::pair<float, DWORD> > & acc, FatVert const & fv);
		void ExportAnimations(ID3DXFileSaveObject * save);
		void ExportFileData(ID3DXFileSaveObject * save);
		template<typename T> void AddKeyValue(T * cont, char const * key, char const * value);

		std::string SettingsFilename();
		void LoadSettings();
		void SaveSettings();

		IGameExporter();
		~IGameExporter();		

	};

	static bool inited = false;


	class IGameExporterClassDesc : public ClassDesc2 {
	public:
		int 			IsPublic() { return TRUE; }
		void *			Create(BOOL loading = FALSE) { return new IGameExporter(); }
		const TCHAR *	ClassName() { return _T("kwXport"); }
		SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
		Class_ID		ClassID() { return KWXPORT_CLASS_ID; }
		const TCHAR* 	Category() { return _T("Export"); }

		const TCHAR*	InternalName() { return _T("kwXport"); }	// returns fixed parsable name (scripter-visible name)
		HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle

	};



	static IGameExporterClassDesc IGameExporterDesc;
	ClassDesc2* GetIGameExporterDesc() { return &IGameExporterDesc; }

	IGameExporter::IGameExporter()
	{
	}

	IGameExporter::~IGameExporter() 
	{
	}

	int IGameExporter::ExtCount()
	{
		return 1;
	}

	const TCHAR *IGameExporter::Ext(int n)
	{		
		return _T("x");
	}

	const TCHAR *IGameExporter::LongDesc()
	{
		return _T("Kilowatt X file exporter.");
	}

	const TCHAR *IGameExporter::ShortDesc() 
	{			
		return _T("kW X-port");
	}

	const TCHAR *IGameExporter::AuthorName()
	{			
		return _T("Jon Watte");
	}

	const TCHAR *IGameExporter::CopyrightMessage() 
	{	
		return _T("");
	}

	const TCHAR *IGameExporter::OtherMessage1() 
	{		
		return _T("");
	}

	const TCHAR *IGameExporter::OtherMessage2() 
	{		
		return _T("");
	}

	unsigned int IGameExporter::Version()
	{				
		return 0 * 100 + 0 * 10 + 1;
	}

	INT_PTR CALLBACK AboutProc(
		HWND hwndDlg,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam)
	{
		if (uMsg == WM_COMMAND) {
			if (LOWORD(wParam) == IDOK) {
				::EndDialog(hwndDlg, 0);
			}
			return TRUE;
		}
		if (uMsg == WM_INITDIALOG) {
			return TRUE;
		}
		return FALSE;
	}


	void IGameExporter::ShowAbout(HWND hWnd)
	{
		::DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
	}

	BOOL IGameExporter::SupportsOptions(int ext, DWORD options)
	{
		return TRUE;
	}



	// Dummy function for progress bar
	DWORD WINAPI fn(LPVOID arg)
	{
		return(0);
	}



	class MyErrorProc : public IGameErrorCallBack
	{
	public:
		void ErrorProc(IGameError error)
		{
			TCHAR * buf = GetLastIGameErrorText();
			if (error != 9) {
				//  I don't care about missing materials on faces
				DebugPrint(_T("ErrorCode = %d ErrorText = %s\n"), error,buf);
			}
		}
	};

	static string convert_name(TCHAR const * name)
	{
		if (name == 0) {
			return string(_T("unnamed"));
		}
		string ret(name);
		if ((name[0] >= '0') && (name[0] <= '9')) {
			ret = std::string("_") + ret;
		}
		for (size_t i = 0; i < ret.size(); ++i) {
			TCHAR ch = ret[i];
			if (((ch >= '0') && (ch <= '9'))
				|| ((ch >= 'A') && (ch <= 'Z'))
				|| ((ch >= 'a') && (ch <= 'z'))) {
					continue;
			}
			ret[i] = '_';
		}
		return ret;
	}

	template<typename T>
	void append(std::vector<char> & v, T const & t)
	{
		v.insert(v.end(), (char const *)&t, ((char const *)&t)+sizeof(T));
	}

	template<typename T>
	void append(std::vector<char> & v, std::vector<T> const & t)
	{
		if (t.size() > 0) {
			v.insert(v.end(), (char const *)&t[0], ((char const *)&t[0])+sizeof(T)*t.size());
		}
	}

	template<typename T>
	void append(std::vector<char> & v, T const * t, size_t cnt)
	{
		v.insert(v.end(), (char const *)t, ((char const *)t)+cnt*sizeof(T));
	}

	template<>
	void append(std::vector<char> & v, std::string const & str)
	{
		v.insert(v.end(), (char const *)str.c_str(), (char const *)str.c_str()+str.size()+1);
	}

	DWORD IGameExporter::VertexAdd(std::vector<FatVert> & vec, std::vector<std::pair<float, DWORD> > & acc, FatVert const & fv)
	{
		size_t n = vec.size();
		size_t i = 0;
		std::pair<float, DWORD> p(fv.pos.x, 0);
		std::pair<float, DWORD> * base = 0;
		if (acc.size() > 0) {
			std::pair<float, DWORD> * low = &acc[0], * high = low+acc.size();
			base = low;
			for (;;) {
				std::pair<float, DWORD> * mid = low + (high-low)/2;
				if (mid == low) {
					i = mid-&acc[0];
					break;
				}
				if (mid->first < p.first) {
					low = mid;
				}
				else {
					high = mid;
				}
			}
		}
		for (; i < n; ++i) {
			if (vec[base[i].second].pos.x > fv.pos.x) {
				//  no need searching more
				i = base[i].second;
				break;
			}
			if (!memcmp(&vec[base[i].second], &fv, sizeof(fv))) {
				return base[i].second;
			}
		}
		p.second = (DWORD)vec.size();
#if !defined(NDEBUG)
		size_t nv = vec.size();
		for (size_t i = 0; i < nv; ++i) {
			assert( memcmp(&fv, &vec[i], sizeof(FatVert)) );
		}
#endif
		acc.push_back(p);
		std::inplace_merge(&acc[0], &acc[acc.size()-1], &acc[0]+acc.size());
		vec.push_back(fv);
		return p.second;
	}

	template<typename T>
	void appendProperty(IGameProperty * prop, std::vector<char> & out)
	{
		float f;
		int i;
		Point3 p3;
		Point4 p4;
		size_t size;

		switch (prop->GetType()) {
	case IGAME_POINT3_PROP:
		prop->GetPropertyValue(p3);
		append(out, p3);
		size = sizeof(p3);
		break;
	case IGAME_POINT4_PROP:
		prop->GetPropertyValue(p4);
		append(out, p4);
		size = sizeof(p4);
		break;
	case IGAME_FLOAT_PROP:
		prop->GetPropertyValue(f);
		append(out, f);
		size = sizeof(f);
		break;
	case IGAME_INT_PROP:
		prop->GetPropertyValue(i);
		append(out, i);
		size = sizeof(i);
		break;
	default:
		::DebugPrint("Property type %d T %s\n", prop->GetType(), typeid(T).name());
		throw "Unknown property kind in appendProperty()\n";
		}
		if (size < sizeof(T)) {
			out.insert(out.end(), sizeof(T)-size, 0);
		}
		else if (size > sizeof(T)) {
			::DebugPrint("Property type %d T %s\n", prop->GetType(), typeid(T).name());
			throw "Too big property in appendProperty()\n";
		}
	}

	class Remap {
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

	void IGameExporter::WriteGeometry2(IGameScene * ig, IGameNode * node, IGameObject * obj, ID3DXFileSaveData * frame)
	{
		__try {
			WriteGeometry(ig, node, obj, frame);
		}
		__except(MiniDumpHandler(GetExceptionInformation())) {
			throw "The exporter crashed during export.\n\n"
				"Please close Max and re-start it before trying to export again.\n"
				"A minidump file called c:\\kwxport.dmp may have been created \n"
				"for further problem analysis.";
		}
	}

	char const * SafeName(IGameNode * node, char const *prefix)
	{
		static char buf[256];
		char const * name = node->GetName();
		if (!name || !name[0]) {
			sprintf(buf, "%s_%lx", prefix, (long)(size_t)node);
			return buf;
		}
		sprintf(buf, "%s_%s", prefix, name);
		return buf;
	}

	void IGameExporter::WriteGeometry(IGameScene * ig, IGameNode * node, IGameObject * obj, ID3DXFileSaveData * frame)
	{
		bool exportNormal = exportNormal_;
		bool exportTangent = exportTangent_;
		int exportUv = exportUv_;
		bool flipV = flipV_;
		bool exportColor = exportColor_;
		bool exportSkinning = exportSkinning_;
		bool exportMaterial = exportMaterial_;
		bool exportShader = exportShader_;
		bool fullPath = fullPath_;
		bool renameTex = renameTex_;
		std::string prefixTex = prefixTex_;

		//  retrieve the appropriate geometry
		std::vector<char> mesh;
		IGameMesh * gm = static_cast<IGameMesh *>(obj);
		IGameSkin * gs = obj->IsObjectSkinned() ? obj->GetIGameSkin() : 0;
#if 0
		if (gs && exportSkinning) {
			//  Documentation says the initial pose is the data before the skin was applied.
			//  However, it actually appears to be the pose at the static frame (?)
			gm = gs->GetInitialPose();
		}
#endif
		gm->SetCreateOptimizedNormalList();
		if (!gm->InitializeData()) {
			static char buf[4096];
			_snprintf(buf, 4095, "Could not read mesh data from mesh '%s'\n",
				SafeName(node, "mesh"));
			buf[4095] = 0;
			throw buf;
		}

		//  When I ask for vertices, the object offset is already 
		//  applied, so always use the node TM.
		float scale = scaleNumer_ / scaleDenom_;
		GMatrix nwm = node->GetWorldTM(coreFrame_);
		nwm.SetRow(3, Point4(nwm.Translation() * scale, 1));
		GMatrix worldToNode = nwm.Inverse();
		GMatrix normalMat = worldToNode;
		//  inverse transpose of node-to-obj
		if (gs && exportSkinning) {
			//  The node transform is already included in the skinned data.
			worldToNode = globalMat;
			normalMat = globalMat;  //  assume matrix is non-scaling orthonormal
		}

		DWORD numFaces = gm->GetNumberOfFaces();
		DWORD numFacesUsed = 0;
		std::vector<DWORD> indices;
		indices.resize(numFaces * 3);
		FatVert fv;
		std::vector<FatVert> fatVerts;
		std::vector<std::pair<float, DWORD> > accel;

		//  don't export texture coordinates that aren't available
		Tab<int> mapNums = gm->GetActiveMapChannelNum();
		if (mapNums.Count() == 0) {
			exportTangent = false;
			exportUv = 0;
		}
		if (mapNums.Count() == 1) {
			if (exportUv > 1) {
				exportUv = 1;
			}
		}
		if (exportUv > 2) {
			::DebugPrint("exportUv is %d; only %d channels will be exported.\n", exportUv, 2);
			exportUv = 2;
		}

		//  extract all unique vertices on a per-face basis
		std::vector<DWORD> materialIndices;
		//  some materials may not be included
		std::vector<std::pair<IGameMaterial *, bool> > materials;
		int maxIx = -1;
		std::vector<bool> includeFace;
		for (int i = 0; i < (int)numFaces; ++i) {
			FaceEx * fex = gm->GetFace(i);
			IGameMaterial * imat = gm->GetMaterialFromFace(fex);
			size_t nMat = materials.size();
			bool inc;
			for (size_t m = 0; m < nMat; ++m) {
				if (materials[m].first == imat) {
					includeFace.push_back(materials[m].second);
					if (!materials[m].second) {
						goto dont_include;
					}
					materialIndices.push_back((DWORD)m);
					goto got_mat;
				}
			}
			// assume DirectX materials are "textured"
			inc = !ignoreUntextured_ || 
				(imat && imat->GetDiffuseData() && imat->GetNumberOfTextureMaps()) || 
				(imat && imat->GetMaxMaterial()->GetInterface(IDXMATERIAL_INTERFACE));
			materials.push_back(std::pair<IGameMaterial *, bool>(imat, inc));
			includeFace.push_back(inc);
			if (!inc) {
				goto dont_include;
			}
			materialIndices.push_back((DWORD)materials.size()-1);
got_mat:
			for (int j = 0; j < 3; ++j) {
				memset(&fv, 0, sizeof(fv));
				fv.origVert = fex->vert[j];
				fv.pos = gm->GetVertex(fex->vert[j], false) * scale * worldToNode;
				GlobalMirror(fv.pos);
#if !defined(NDEBUG)
				DebugPrint("v %.2f,%.2f,%.2f\n", fv.pos.x, fv.pos.y, fv.pos.z);
#endif
				//  additional fields for each vertex based on options
				if (exportNormal) {
					fv.normal = TransformVector(normalMat, gm->GetNormal(fex->norm[j], false));
#if !defined(NDEBUG)
					DebugPrint("n %.3f,%.3f,%.3f\n", fv.normal.x, fv.normal.y, fv.normal.z);
#endif
					GlobalMirror(fv.normal);
				}
				if (exportTangent) {
					fv.tangent = TransformVector(normalMat, gm->GetTangent(fex->norm[j], 1));
					fv.binormal = TransformVector(normalMat, gm->GetBinormal(fex->norm[j], 1));
					GlobalMirror(fv.tangent);
					GlobalMirror(fv.binormal);
				}
				for (int q = 0; q < exportUv; ++q) {
					DWORD index[3] = {0, 0, 0};
					gm->GetMapFaceIndex(mapNums[q], i, index);
					fv.uv[q] = (Point2&)gm->GetMapVertex(mapNums[q], index[j]);
					//  The default is a need to flip the texture, at least 
					//  when it's a JPG or similar.
					if (!flipV_) {
						fv.uv[q].y = 1.0f - fv.uv[q].y;
					}
				}
				if (exportColor) {
					int nColor = gm->GetNumberOfColorVerts();
					int nAlpha = gm->GetNumberOfAlphaVerts();
					Point3 p3(1, 1, 1);
					int ix = fex->color[j];
					if (ix >= 0 && ix < nColor) {
						p3 = gm->GetColorVertex(fex->color[j]);
					}
					Point4 color(p3.x, p3.y, p3.z, 1.0f);
					ix = fex->alpha[j];
					if (ix >= 0 && ix < nAlpha) {
						color.w = gm->GetAlphaVertex(fex->alpha[j]);
					}
					fv.color = color;
				}
				indices[numFacesUsed * 3 + j] = VertexAdd(fatVerts, accel, fv);
			}
			++numFacesUsed;
dont_include:
			;
		}
		indices.resize(numFacesUsed*3);
		if (flipZ_) {
			for (size_t i = 0; i < numFacesUsed; ++i) {
				std::swap(indices[i*3], indices[i*3+1]);
			}
		}

		//  generate the initial Mesh geometry data
		DWORD numVertices = (DWORD)fatVerts.size();
		if (numVertices == 0) {
			return; //  no idea trying to add anything else
		}
		append(mesh, numVertices);
		std::vector<Point3> verts;
		verts.resize(numVertices);
		for (int i = 0; i < (int)numVertices; ++i) {
			verts[i] = fatVerts[i].pos;
		}
		append(mesh, verts);
		append(mesh, numFacesUsed);
		//  re-pack indices to include a "3" in front of each triangle
		DWORD tri[4];
		tri[0] = 3;
		DWORD curUsedIx = 0;
		for (int i = 0; i < (int)numFaces; ++i) {
			if (includeFace[i]) {
				for (int j = 0; j < 3; ++j) {
					tri[j+1] = indices[curUsedIx*3+j];
				}
				append(mesh, tri);
				++curUsedIx;
			}
		}
		assert(curUsedIx == numFacesUsed);

		CComPtr<ID3DXFileSaveData> stored;
		CHECK( frame->AddDataObject(TID_D3DRMMesh, SafeName(node, "mesh"), 0, mesh.size(), &mesh[0], &stored) );

		//  Calculate mapping from old vertex index to new vertex indices
		Remap remap;
		if (fatVerts.size() > 0) {
			FatVert * fvp = &fatVerts[0], * base = fvp, * end = fvp + fatVerts.size();
			int mo = -1;
			while (fvp < end) {
				if (mo < fvp->origVert) {
					mo = fvp->origVert;
				}
				++fvp;
			}
			remap.SetOldCount(mo + 1);
			fvp = base;
			while (fvp < end) {
				remap.MapOldToNew((int)fvp->origVert, (int)(fvp-base));
				++fvp;
			}
		}

		//  write normals
		if (exportNormal) {
			std::vector<char> normals;
			DWORD numNormals = (DWORD)fatVerts.size();
			append(normals, numNormals);
			verts.resize(numNormals);
			for (int i = 0; i < (int)numNormals; ++i) {
				verts[i] = fatVerts[i].normal;
			}
			append(normals, verts);
			append(normals, numFacesUsed);
			curUsedIx = 0;
			for (int i = 0; i < (int)numFaces; ++i) {
				if (includeFace[i]) {
					for (int j = 0; j < 3; ++j) {
						tri[j+1] = indices[curUsedIx*3+j];
					}
					append(normals, tri);
					++curUsedIx;
				}
			}
			assert(curUsedIx == numFacesUsed);
			CComPtr<ID3DXFileSaveData> sNormals;
			CHECK( stored->AddDataObject(TID_D3DRMMeshNormals, _T("normals"), 0, normals.size(), &normals[0], &sNormals) );
		}

		//  write first texcoords
		if (exportUv > 0) {
			std::vector<char> texcoords;
			DWORD numTexCoords = numVertices;
			append(texcoords, numTexCoords);
			std::vector<Point2> tex;
			tex.resize(numTexCoords);
			for (int i = 0; i < (int)numTexCoords; ++i) {
				tex[i] = fatVerts[i].uv[0];
			}
			append(texcoords, tex);
			CComPtr<ID3DXFileSaveData> sTexcoords;
			CHECK( stored->AddDataObject(TID_D3DRMMeshTextureCoords, _T("tc0"), 0, texcoords.size(), &texcoords[0], &sTexcoords) );
		}

		//  write vertex colors
		if (exportColor) {
			std::vector<char> colors;
			DWORD numVertexColors = numVertices;
			append(colors, numVertexColors);
			for (int i = 0; i < (int)numVertexColors; ++i) {
				DWORD target = i;
				append(colors, target);
				append(colors, fatVerts[i].color);
			}
			CComPtr<ID3DXFileSaveData> sColors;
			CHECK( stored->AddDataObject(TID_D3DRMMeshVertexColors, _T("col0"), 0, colors.size(), &colors[0], &sColors) );
		}

		// write tangent basis and tc1
		if (exportUv > 1 || exportTangent) {
			D3DVERTEXELEMENT9 elems[4] = { {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}, {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}, {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}, {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}};
			int i = 0;
			WORD offset = 0;
			if (exportUv > 1) {
				D3DVERTEXELEMENT9 ve = { 0, offset, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 };
				elems[i] = ve;
				++i;
				offset += 8;
			}
			if (exportTangent) {
				D3DVERTEXELEMENT9 ve1 = { 0, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0 };
				offset += 12;
				D3DVERTEXELEMENT9 ve2 = { 0, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 };
				offset += 12;
				elems[i] = ve1;
				++i;
				elems[i] = ve2;
				++i;
			}
			std::vector<char> data;
			append(data, (DWORD)i);
			for (int j = 0; j < i; ++j) {
				append(data, (DWORD)elems[j].Type);
				append(data, (DWORD)elems[j].Method);
				append(data, (DWORD)elems[j].Usage);
				append(data, (DWORD)elems[j].UsageIndex);
			}
			DWORD dSize = offset/4 * numVertices;
			append(data, dSize);
			for (DWORD d = 0; d < numVertices; ++d) {
				if (exportUv > 1) {
					append(data, (Point2 const &)fatVerts[d].uv[1]);
				}
				if (exportTangent) {
					append(data, (Point3 const &)fatVerts[d].binormal);
					append(data, (Point3 const &)fatVerts[d].tangent);
				}
			}
			CComPtr<ID3DXFileSaveData> fsd;
			CHECK( stored->AddDataObject(DXFILEOBJ_DeclData, 0, 0, data.size(), &data[0], &fsd) );
		}

		//todo: write morph targets if present

		//  write skinning data if present
		if (gs && exportSkinning) {
			//  build a table from vertex to array of bones
			struct BoneWeight {
				int index;
				float weight;
				bool operator==(BoneWeight const & o) const {
					return o.weight == weight;
				}
				bool operator<(BoneWeight const & o) const {
					//  sort biggest first
					return o.weight < weight;
				}
			};
			//  build the cross-mapping of verticies used by bones
			std::vector<std::vector<BoneWeight> > assignments;
			std::map<IGameNode *, int> boneIndices;
			std::map<IGameNode *, int>::iterator ptr;
			int vc = gs->GetNumOfSkinnedVerts();
			DebugPrint("%d skinned vertices in %s\n", vc, node->GetName());
			//  If a single vertex is weighted to more than 40 bones, 
			//  truncate at 40.
			BoneWeight bw[40];
			int numMax = 0;
			for (int vi = 0; vi < vc; ++vi) {
				int nb = gs->GetNumberOfBones(vi);
				if (nb > 40) {
					nb = 40;
				}
				for (int i = 0; i < nb; ++i) {
					IGameNode * node = gs->GetIGameBone(vi, i);
					ptr = boneIndices.find(node);
					if (ptr != boneIndices.end()) {
						bw[i].index = (*ptr).second;
					}
					else {
						bw[i].index = (int)boneIndices.size();
						boneIndices[node] = bw[i].index;
						assignments.push_back(std::vector<BoneWeight>());
					}
					bw[i].weight = gs->GetWeight(vi, i);
				}
				std::sort(bw, &bw[nb]);
				if (nb < 4) {
					memset(&bw[nb], 0, (4-nb)*sizeof(BoneWeight));
					if (nb > numMax) {
						numMax = nb;
					}
				}
				else {
					numMax = 4;
				}
				//  normalize the 4 highest weighted
				float sum = bw[0].weight + bw[1].weight + bw[2].weight + bw[3].weight;
				sum = 1.0f / sum;
				bw[0].weight *= sum; bw[1].weight *= sum; bw[2].weight *= sum; bw[3].weight *= sum;
				for (int i = 0; i < 4; ++i) {
					BoneWeight ww;
					ww.index = vi;
					ww.weight = bw[i].weight;
					//  don't include a vert that's not actually affected
					if (ww.weight > 0) {
						assignments[bw[i].index].push_back(ww);
					}
				}
			}
			//  calculate number of bones that affect the mesh
			int numAffect = 0;
			for (size_t as = 0; as < assignments.size(); ++as) {
				if (assignments[as].size() > 0) {
					++numAffect;
				}
			}

			//  write XSkinMeshHeader data
			CComPtr<ID3DXFileSaveData> skinHeader;
			struct hdr {
				WORD weights_per_vertex;
				WORD weights_per_face;
				WORD num_bones;
			};
			hdr h;
			h.weights_per_vertex = numMax;
			h.weights_per_face = 0;
			h.num_bones = numAffect;
			CHECK( stored->AddDataObject(DXFILEOBJ_XSkinMeshHeader, 0, 0, 12, &h, &skinHeader) );

			//  write an XSkinInfo for each bone that affects the mesh
			for (std::map<IGameNode *, int>::iterator ptr = boneIndices.begin(), end = boneIndices.end();
				ptr != end; ++ptr) {
					IGameNode * bone = (*ptr).first;
					std::vector<char> skinData;
					std::string s(convert_name(bone->GetName()));
					skinData.insert(skinData.end(), s.c_str(), s.c_str()+s.length()+1);
					int bix = (*ptr).second;
					std::vector<BoneWeight> & bwv = assignments[bix];
					if (bwv.size()) {
						std::vector<DWORD> skinIndices;
						std::vector<float> skinWeights;
						BoneWeight * bw = &bwv[0];
						DWORD n = (DWORD)bwv.size();
						for (DWORD t = 0; t < n; ++t) {
							int oix = bw[t].index;
							float ow = bw[t].weight;
							//  Each old vert may generate more than one new vert
							for (int i = 0; i < 8; ++i) {
								int x = remap.GetNew(oix, i);
								if (x == -1) break;
								assert(x < (int)numVertices && x >= 0);
								skinIndices.push_back(x);
								skinWeights.push_back(ow);
							}
						}
						append(skinData, (DWORD)skinIndices.size());
						append(skinData, skinIndices);
						append(skinData, skinWeights);
						//  Calculate bone matrix at init time.
						//  Then calculate the matrix that takes the init vertex to the 
						//  init bone space.
#if 0
						GMatrix boneInitTM;
						if (!gs->GetInitBoneTM(bone, boneInitTM)) {
							DebugPrint("Bone %s doesn't have an initial TM?\n", bone->GetName());
							boneInitTM = bone->GetWorldTM();
						}
#else
						GMatrix boneInitTM = bone->GetWorldTM(coreFrame_) * globalMat;
#endif
						//  The vertices are actually in identity space already; the 
						//  skin init TM would change the vertex position to world space.
						//  However, the bone matrix goes from bone space to world space, 
						//  so invert that.
						GlobalMirror(boneInitTM);
						GMatrix gm = boneInitTM.Inverse();
						assert(sizeof(gm) == 16*4);
						gm.SetRow(3, Point4(gm.Translation() * scale, 1));
						append(skinData, gm);
						CComPtr<ID3DXFileSaveData> saveSkin;
						CHECK( stored->AddDataObject(DXFILEOBJ_SkinWeights, (std::string("W-") + s).c_str(), 0, skinData.size(), &skinData[0], &saveSkin) );
					}
			}
		}

		//  write materials
		if (exportMaterial) {
			std::vector<char> material;
			append(material, (DWORD)materials.size());
			append(material, (DWORD)materialIndices.size());
			append(material, materialIndices);
			CComPtr<ID3DXFileSaveData> mlist;
			CHECK( stored->AddDataObject(TID_D3DRMMeshMaterialList, _T("mtls"), 0, material.size(), &material[0], &mlist) );
			//  write textures
			for (size_t i = 0; i < materials.size(); ++i) {
				material.resize(0);
				//  extract parameter block data
				float f;
				Point3 p3(0,0,0);
				Point4 p4(0,0,0,1);
				std::string mname;
				if (!materials[i].first) {
not_stdmat:
					IPoint3 ip3 = node->GetWireframeColor();
					append(material, Point4(ip3.x / 255.0f, ip3.y / 255.0f, ip3.z / 255.0f, 1.0f));
					append(material, 16.0f);
					append(material, Point3(1, 1, 1));
					append(material, Point3(0, 0, 0));
					mname = "Dflt_Material";
				}
				else {
					IGameMaterial * theMat = materials[i].first;
					IGameProperty * matProp = theMat->GetDiffuseData();
					if (!matProp) goto not_stdmat;
					int type = matProp->GetType();
					assert(type == IGAME_POINT3_PROP);
					matProp->GetPropertyValue((Point3&)p4);
					matProp = theMat->GetOpacityData();
					if (!matProp) goto not_stdmat;
					type = matProp->GetType();
					assert(type == IGAME_FLOAT_PROP);
					matProp->GetPropertyValue(p4.w);
					append(material, p4);
					matProp = theMat->GetGlossinessData();
					if (!matProp) goto not_stdmat;
					type = matProp->GetType();
					assert(type == IGAME_FLOAT_PROP);
					matProp->GetPropertyValue(f);
					f *= 100;
					append(material, f);
					matProp = theMat->GetSpecularData();
					if (!matProp) goto not_stdmat;
					matProp->GetPropertyValue(p3);
					type = matProp->GetType();
					assert(type == IGAME_POINT3_PROP);
					matProp = theMat->GetSpecularLevelData();
					if (!matProp) goto not_stdmat;
					matProp->GetPropertyValue(f);
					if (f > 0) {
						p3.x = f; p3.y = f; p3.z = f;
					}
					append(material, p3);
					matProp = theMat->GetEmissiveData();
					if (!matProp) goto not_stdmat;
					matProp->GetPropertyValue(p3);
					type = matProp->GetType();
					assert(type == IGAME_POINT3_PROP);
					matProp = theMat->GetEmissiveAmtData();
					if (!matProp) goto not_stdmat;
					matProp->GetPropertyValue(f);
					if (f > 0) {
						p3.x = f; p3.y = f; p3.z = f;
					}
					append(material, p3);
					mname = convert_name(theMat->GetMaterialName());
				}

				CComPtr<ID3DXFileSaveData> mtl;
				CHECK( mlist->AddDataObject(TID_D3DRMMaterial, mname.c_str(), 
					0, material.size(), &material[0], &mtl) );

				//  write texture map names
				int nText = materials[i].first ? materials[i].first->GetNumberOfTextureMaps() : 0;
				std::map<string, string> textures;
				for (int bi = 0; bi < nText; ++bi) {
					IGameTextureMap * gtx = materials[i].first->GetIGameTextureMap(bi);
					int slot = gtx->GetStdMapSlot();
					if (slot < 0 || slot >= sizeof(mapSlotNames)/sizeof(mapSlotNames[0])) {
						slot = sizeof(mapSlotNames)/sizeof(mapSlotNames[0])-1;
					}
					std::string texPath = gtx->GetBitmapFileName();
					if (renameTex) {
						std::string::size_type st = texPath.rfind('.');
						if (st != texPath.npos) {
							texPath.replace(st, texPath.size()-st, ".dds");
						}
					}
					TCHAR const * slash = texPath.c_str();
					if (!fullPath) {
						slash = strrchr(texPath.c_str(), '\\');
						if (!slash) slash = strrchr(texPath.c_str(), '/');
						if (!slash) slash = texPath.c_str(); else ++slash;
						if (*slash) {
							std::string s = prefixTex_ + slash;
							texPath = s;
							slash = texPath.c_str();
						}
					}
					if (slash && slash[0]) {
						textures[mapSlotNames[slot]] = slash;
					}
					if (slot == ID_DI) {
						if (slash && slash[0]) {
							CComPtr<ID3DXFileSaveData> tex;
							CHECK( mtl->AddDataObject(TID_D3DRMTextureFilename, "Diffuse", 0, strlen(slash)+1, slash, &tex) );
						}
					}
				}
				//  write an effect
				IGameFX * gfx = NULL;
				if (i < materials.size() && materials[i].first != NULL) {
					gfx = materials[i].first->GetIGameFX();
				}
				if (gfx && exportShader) {
					std::string fxPath = gfx->GetEffectFileName();
					TCHAR const * slash = fxPath.c_str();
					if (!fullPath) {
						slash = strrchr(fxPath.c_str(), '\\');
						if (!slash) slash = strrchr(fxPath.c_str(), '/');
						if (!slash) slash = fxPath.c_str(); else ++slash;
						if (*slash) {
							std::string s = prefixTex_ + slash;
							fxPath = s;
							slash = fxPath.c_str();
						}
					}
					if (slash && slash[0]) {
						CComPtr<ID3DXFileSaveData> fxData;
						CHECK( mtl->AddDataObject(DXFILEOBJ_EffectInstance, "Effect", 0, strlen(slash)+1, slash, &fxData) );
						//  write effect parameters
						int nProp = gfx->GetNumberOfProperties();
						for (int fxi = 0; fxi < nProp; ++fxi) {
							IGameFXProperty * gp = gfx->GetIGameFXProperty(fxi);
							std::string semantic = gp->GetFXSemantic();
							std::string name = gp->GetPropertyName();
							std::string dflt = gp->GetFXDefaultValue();
							std::string type = gp->GetPropertyType();
							std::string val;
							IGameProperty * prop = gp->GetIGameProperty();
							float fdata[4];
							int i;
							TCHAR * str;
							int wrFloat = 0;
							bool wrDword = false;
							bool wrString = false;
							char buf[50];
							switch (prop ? prop->GetType() : IGAME_UNKNOWN_PROP) {
	case IGAME_FLOAT_PROP:
		prop->GetPropertyValue(fdata[0]);
		sprintf(buf, "%.f", fdata[0]);
		val = buf;
		wrFloat = 1;
		break;
	case IGAME_INT_PROP:
		prop->GetPropertyValue(i);
		sprintf(buf, "%d", i);
		val = buf;
		wrDword = true;
		break;
	case IGAME_POINT3_PROP:
		prop->GetPropertyValue((Point3 &)fdata[0]);
		sprintf(buf, "float3(%f,%f,%f)", fdata[0], fdata[1], fdata[2]);
		val = buf;
		wrFloat = 3;
		break;
	case IGAME_POINT4_PROP:
		prop->GetPropertyValue((Point4 &)fdata[0]);
		sprintf(buf, "float4(%f,%f,%f,%f)", fdata[0], fdata[1], fdata[2], fdata[3]);
		val = buf;
		wrFloat = 4;
		break;
	case IGAME_STRING_PROP:
		prop->GetPropertyValue(str);
		val = str;
		wrString = true;
		break;
	case IGAME_UNKNOWN_PROP:
		switch (gp->GetPropertyUsage()) {
	case IGameFXProperty::IGAMEFX_GENERAL:
	case IGameFXProperty::IGAMEFX_LIGHT:
	case IGameFXProperty::IGAMEFX_SAMPLER:
		break;
	case IGameFXProperty::IGAMEFX_TEXTURE: {
		if (prop->IsPBlock2()) {
			IParamBlock2 * pb2 = prop->GetMaxParamBlock2();
			Interval ivalid;
			BOOL b;
			int id = prop->GetParamBlockIndex();
			ParamDef const & pd = pb2->GetParamDef(id);
			if (pb2->GetParameterType(id) == TYPE_STRING || pb2->GetParameterType(0) == TYPE_FILENAME) {
				b = pb2->GetValue(0, 0, str, ivalid);
				if (b) {
					val = str;
					wrString = val.size() > 0;
				}
			}
			else if (pb2->GetParameterType(id) == TYPE_TEXMAP) {
				Texmap * tmp = 0;
				b = pb2->GetValue(id, 0, tmp, ivalid, 0);
				BitmapTex * bmt = 0;
				if (b && tmp && tmp->ClassID().PartA() == BMTEX_CLASS_ID) {
					bmt = static_cast<BitmapTex *>(tmp);
				}
				if (bmt) {
					val = (str = bmt->GetMapName());
					wrString = val.size() > 0;
				}
			}
			else if (pb2->GetParameterType(id) == TYPE_BITMAP) {
				PBBitmap * pbb = 0;
				b = pb2->GetValue(id, 0, pbb, ivalid, 0);
				val = (str = (TCHAR *)pbb->bi.Name());
				wrString = val.size() > 0 && strcmp(str, "None");
			}
		}
		break; }
		}
		break;
							}
							if (wrFloat || wrDword || wrString) {
								::OutputDebugString((type + " " + name + (semantic.size() ? " : " : "") + semantic 
									+ " = " + val + (dflt.size() ? " ; default " : " ;") + dflt + "\n").c_str());
							}
							std::vector<char> data;
							if (wrFloat) {
								CComPtr<ID3DXFileSaveData> propData;
								data.resize(0);
								append(data, name);
								append(data, wrFloat);
								append(data, fdata, wrFloat);
								CHECK( fxData->AddDataObject(DXFILEOBJ_EffectParamFloats, 0, 0, data.size(), &data[0], &propData) );
							}
							if (wrDword) {
								CComPtr<ID3DXFileSaveData> propData;
								data.resize(0);
								append(data, name);
								append(data, i);
								assert(sizeof(i) == sizeof(DWORD));
								CHECK( fxData->AddDataObject(DXFILEOBJ_EffectParamDWord, 0, 0, data.size(), &data[0], &propData) );
							}
							if (wrString) {
								CComPtr<ID3DXFileSaveData> propData;
								data.resize(0);
								append(data, name);
								//  This is a hack to recognize absolute paths, and 
								//  turn them into relative paths if we don't want absolutes.
								if (val.size() > 3 && val[1] == ':' && val[2] == '\\' && !fullPath) {
									std::string::size_type pos = val.rfind('\\');
									if (pos != std::string::npos) {
										val.erase(0, pos+1);
										val = prefixTex + val;
									}
								}
								append(data, val);
								CHECK( fxData->AddDataObject(DXFILEOBJ_EffectParamString, 0, 0, data.size(), &data[0], &propData) );
							}
						}
					}
				}
			}
		}
	}

	template<typename T>
	void IGameExporter::ExportNode(IGameScene * ig, IGameNode * node, T * parent)
	{
		animated_.push_back(node);

		GMatrix otm = node->GetWorldTM(coreFrame_) * globalMat;
		GMatrix ptm;
		ptm.SetIdentity();
		if (node->GetNodeParent() != 0) {
			ptm = node->GetNodeParent()->GetWorldTM(coreFrame_) * globalMat;
			//  otm is local relative to parent
			otm = otm * ptm.Inverse();
		}

		otm.SetRow(3, Point4(otm.Translation() * scaleNumer_ / scaleDenom_, 1));
		GlobalMirror(otm);

		CComPtr<ID3DXFileSaveData> frame;
		std::string n = convert_name(node->GetName());
		DebugPrint(_T("Saving node: %s\n"), n.c_str());
		CHECK( parent->AddDataObject(TID_D3DRMFrame, n.c_str(), 0, 0, 0, &frame) );

		//  todo: set transformation to identity if "trim parents" is turned on

		CComPtr<ID3DXFileSaveData> temp;
		CHECK( frame->AddDataObject(TID_D3DRMFrameTransformMatrix, "relative", 0, sizeof(otm), &otm, &temp) );
		temp = 0;

		//  todo: save user props, if chosen
		if (exportComments_) {
			TSTR uPropBuf;
			node->GetMaxNode()->GetUserPropBuffer(uPropBuf);
			TCHAR const *str = uPropBuf.data();
			TCHAR const *end = str + uPropBuf.length();
			TCHAR const *base = str;
			TCHAR const *eq = 0;
			while (str < end) {
				if (base == 0 && !isspace(*str)) {
					base = str;
				}
				if (*str == '\n' || *str == '\r') {
store:
					//  treat '#' as a comment field
					if (eq != 0 && eq > base && base < str && *base != '#') {
						std::string key(base, eq);
						std::string value(eq+1, str);
						AddKeyValue( frame.p, key.c_str(), value.c_str() );
					}
					eq = 0;
					base = 0;
				}
				else if (str == end) {
					++str;
					goto store;
				}
				else if (*str == '=') {
					eq = str;
				}
				++str;
			}
		}

		GMatrix nmat2 = (node->GetWorldTM(coreFrame_) * globalMat).Inverse();
		GMatrix omat = (node->GetObjectTM(coreFrame_) * globalMat) * nmat2;
		GlobalMirror(omat);
		CHECK( frame->AddDataObject(kW_ObjectMatrixComment, "object", 0, sizeof(omat), &omat, &temp) );
		temp = 0;

		IGameObject * obj = node->GetIGameObject();
		if (obj) {
			if (obj->GetIGameType() == IGameObject::IGAME_SPLINE) {
				//  This test -- before checking whether it's supported -- is a 
				//  necessary work-around to avoid crashing, due to a bug in 3ds Max 3DXI.
			}
			else if (obj->IsEntitySupported()) {
				switch (obj->GetIGameType()) {
	case IGameObject::IGAME_MESH:
		WriteGeometry2(ig, node, obj, frame);
		break;
		//todo: write cameras, lights and dummies
	default:
		// do nothing
		break;
				}
			}
		}

		int nc = node->GetChildCount();
		for (int i = 0; i < nc; ++i) {
			ExportNode(ig, node->GetNodeChild(i), frame.p);
		}
	}

	static bool almostEqual(Point3 const & a, Point3 const & b)
	{
		return fabsf(a.x-b.x) < 1e-4 && fabsf(a.y-b.y) < 1e-4 && fabsf(a.z-b.z) < 1e-4;
	}

	static bool almostEqual(Quat const & a, Quat const & b)
	{
		return fabsf(a.x-b.x) < 1e-4 && fabsf(a.y-b.y) < 1e-4 && fabsf(a.z-b.z) < 1e-4 && fabsf(a.w-b.w) < 1e-4;
	}

	static float dot(Quat const & q, Quat const & p)
	{
		return q.x*p.x + q.y*p.y + q.z*p.z + q.w*p.w;
	}

	void IGameExporter::ExportAnimations(ID3DXFileSaveObject * save)
	{
		float scale = scaleNumer_ / scaleDenom_;
		std::vector<AnimationRange> vec;
		GetAnimationRanges(allAnimations_, vec);
		AnimationRange ar;
		ar.first = firstFrame_;
		ar.length = numFrames_;
		ar.name = animationName_;
		ar.timeStretch = timeStretch_;
		if (vec.size() == 0) {
			//  If there are no animations configured, use the 
			//  settings from the settings fields.
			vec.push_back(ar);
		}
		for (std::vector<AnimationRange>::iterator arp = vec.begin(), are = vec.end();
			arp != are; ++arp) {
				ar = *arp;
				std::vector<IGameNode *>::iterator ptr, end;
				CComPtr<ID3DXFileSaveData> animSet;
				DWORD dwd = 4800;   //  3dsMax tick rate is 4800 ticks per second
				CComPtr<ID3DXFileSaveData> anim(0), temp(0);
				CHECK( save->AddDataObject(DXFILEOBJ_AnimTicksPerSecond, "fps", 0, 4, &dwd, &temp) );
				dwd /= GetFrameRate();
				DWORD outTime = (DWORD)(dwd * ar.timeStretch);
				if (outTime < 0) outTime = 0;
				temp = 0;
				CHECK( save->AddDataObject(TID_D3DRMAnimationSet, ar.name.c_str(), 0, 0, 0, &animSet) );
				float scale = scaleNumer_ / scaleDenom_;
				for (ptr = animated_.begin(), end = animated_.end(); ptr != end; ++ptr) {
					IGameNode * node = *ptr;
					std::vector<std::pair<std::pair<DWORD, DWORD>, Point3> > posAnim;
					std::vector<std::pair<std::pair<DWORD, DWORD>, Point3> > scaleAnim;
					std::vector<std::pair<std::pair<DWORD, DWORD>, Quat> > quatAnim;
					Point3 op(-101010, 101010, -101010), os(0,0,0);
					Quat oq(1.0f, 1.0f, 1.0f, 1.0f);
					//  Remember how many frames were dropped, so that the last keyframe 
					//  before a change can be put back in.
					int compPos = 0;
					int compRot = 0;
					int compScale = 0;
					for (int time = 0; time < ar.length; ++time) {
						GMatrix ip;
						IGameNode * np = node->GetNodeParent();
						if (np) {
							ip = (np->GetWorldTM(TimeValue((time + ar.first)*dwd)) * globalMat).Inverse();
						}
						else {
							ip.SetIdentity();
						}
						GMatrix tm = node->GetWorldTM(TimeValue((time + ar.first)*dwd)) * globalMat * ip;
						GlobalMirror(tm);
						//  decompose
						Matrix3 m3 = tm.ExtractMatrix3();
						Point3 p;
						Quat q;
						Point3 s;
						DecomposeMatrix(m3, p, q, s);
						//  add components
						if (!almostEqual(p, op) || time == 0) {
							if (compPos > 0) {
								posAnim.push_back(std::pair<std::pair<DWORD, DWORD>, Point3>(
									std::pair<DWORD, DWORD>((time-1)*outTime, 3), op * scale));
							}
							posAnim.push_back(std::pair<std::pair<DWORD, DWORD>, Point3>(
								std::pair<DWORD, DWORD>(time*outTime, 3), p * scale));
							op = p;
							compPos = 0;
						}
						else {
							++compPos;
						}
						if (!almostEqual(s, os) || time == 0) {
							if (compScale > 0) {
								scaleAnim.push_back(std::pair<std::pair<DWORD, DWORD>, Point3>(
									std::pair<DWORD, DWORD>((time-1)*outTime, 3), os));
							}
							scaleAnim.push_back(std::pair<std::pair<DWORD, DWORD>, Point3>(
								std::pair<DWORD, DWORD>(time*outTime, 3), s));
							os = s;
							compScale = 0;
						}
						else {
							++compScale;
						}
						if (dot(q, oq) < 0) {
							//  take the short way around
							q = Quat(-q.x, -q.y, -q.z, -q.w);
						}
						if (!almostEqual(q, oq) || time == 0) {
							//  X files store quats the old way (W first)
							if (compRot > 0) {
								quatAnim.push_back(std::pair<std::pair<DWORD, DWORD>, Quat>(
									std::pair<DWORD, DWORD>((time-1)*outTime, 4), Quat(oq.w, oq.x, oq.y, oq.z)));
							}
							quatAnim.push_back(std::pair<std::pair<DWORD, DWORD>, Quat>(
								std::pair<DWORD, DWORD>(time*outTime, 4), Quat(q.w, q.x, q.y, q.z)));
							oq = q;
							compRot = 0;
						}
						else {
							++compRot;
						}
					}
					anim = 0;
					CHECK( animSet->AddDataObject(TID_D3DRMAnimation, (std::string("Anim-") + convert_name(ar.name.c_str()) + 
						"-" + convert_name((*ptr)->GetName())).c_str(), 0, 0, 0, &anim) );
					CHECK( anim->AddDataReference(convert_name((*ptr)->GetName()).c_str(), 0) );
					DWORD options[2] = { 1, 0 };  // for linear positions
					CComPtr<ID3DXFileSaveData> temp;
					CHECK( anim->AddDataObject(TID_D3DRMAnimationOptions, 0, 0, 8, options, &temp) );
					std::vector<char> foo;
					append(foo, (DWORD)0); // rotation animation
					append(foo, (DWORD)quatAnim.size());
					append(foo, quatAnim);
					temp = 0;
					CHECK( anim->AddDataObject(TID_D3DRMAnimationKey, "rot", 0, foo.size(), &foo[0], &temp) );
					temp = 0;
					foo.clear();
					append(foo, (DWORD)1); // scale animation
					append(foo, (DWORD)scaleAnim.size());
					append(foo, scaleAnim);
					CHECK( anim->AddDataObject(TID_D3DRMAnimationKey, "scale", 0, foo.size(), &foo[0], &temp) );
					temp = 0;
					foo.clear();
					append(foo, (DWORD)2); // position animation
					append(foo, (DWORD)posAnim.size());
					append(foo, posAnim);
					CHECK( anim->AddDataObject(TID_D3DRMAnimationKey, "pos", 0, foo.size(), &foo[0], &temp) );
				}
		}
	}

	// {95A48E28-7EF4-4419-A16A-BA9DBDF0D2BC}
	static const GUID kW_ObjectMatrixComment = 
	{ 0x95a48e28, 0x7ef4, 0x4419, { 0xa1, 0x6a, 0xba, 0x9d, 0xbd, 0xf0, 0xd2, 0xbc } };
	// {26E6B1C3-3D4D-4a1d-A437-B33668FFA1C2}
	static const GUID kW_KeyValuePair = 
	{ 0x26e6b1c3, 0x3d4d, 0x4a1d, { 0xa4, 0x37, 0xb3, 0x36, 0x68, 0xff, 0xa1, 0xc2 } };

	char const * kwTemplates =
		"xof 0303txt 0032\n"
		"\n" 
		"template ObjectMatrixComment {\n"
		"  <95A48E28-7EF4-4419-A16A-BA9DBDF0D2BC>\n"
		"  Matrix4x4 objectMatrix;\n"
		"}\n"
		"\n"
		"template KeyValuePair {\n"
		"  <26E6B1C3-3D4D-4a1d-A437-B33668FFA1C2>\n"
		"  string key;\n"
		"  string value;\n"
		"}\n"
		"\n"
		;

	template<typename T>
	void IGameExporter::AddKeyValue(T * cont, char const * key, char const * value)
	{
		std::vector<char> data;
		append(data, key, strlen(key)+1);
		append(data, value, strlen(value)+1);
		CComPtr<ID3DXFileSaveData> temp;
		CHECK( cont->AddDataObject(kW_KeyValuePair, 0, 0, data.size(), &data[0], &temp) );
	}

	void IGameExporter::ExportFileData(ID3DXFileSaveObject * file)
	{
		CComPtr<ID3DXFileSaveData> data;
		AddKeyValue(file, "Date", exportTime_);
		AddKeyValue(file, "File", core_->GetCurFilePath().data());
		char uName[256] = "";
		DWORD bSize = 256;
		::GetUserName(uName, &bSize);
		uName[255] = 0;
		AddKeyValue(file, "User", uName);
		char buf[100];
		sprintf(buf, "%ld", (long)coreFrame_);
		AddKeyValue(file, "CoreTime", buf);
	}

	void IGameExporter::WriteXFile(IGameScene * ig, TCHAR const * name)
	{
		animated_.clear();
		CComPtr<ID3DXFile> file;
		CHECK( D3DXFileCreate(&file) );
		static char const * xExtensions = XEXTENSIONS_TEMPLATES;
		static char const * xSkinExp = XSKINEXP_TEMPLATES;
		CHECK( file->RegisterTemplates((void*)D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES) );
		CHECK( file->RegisterTemplates((void*)xExtensions, strlen(xExtensions)) );
		CHECK( file->RegisterTemplates((void*)xSkinExp, strlen(xSkinExp)) );
		CHECK( file->RegisterTemplates((void*)kwTemplates, strlen(kwTemplates)) );
		CComPtr<ID3DXFileSaveObject> save;
		D3DXF_FILEFORMAT fileFormat = D3DXF_FILEFORMAT_TEXT;
		if (exportBinary_) {
			fileFormat = D3DXF_FILEFORMAT_BINARY | D3DXF_FILEFORMAT_COMPRESSED;
		}
		CHECK( file->CreateSaveObject(name, D3DXF_FILESAVE_TOFILE, fileFormat, &save) );
		CComPtr<ID3DXFileSaveData> root;
		TCHAR const * slash = strrchr(name, '\\');
		if (!slash) slash = strrchr(name, '/');
		if (slash) ++slash; else slash = name;
		string n = convert_name(slash);

		ExportFileData(save.p);

		//  Sometimes you may wish to keep a single frame at the top. However, 
		//  to preserve compatibility with previous exporters, put each top-level 
		//  frame at the top of the file.
		//  CHECK( save->AddDataObject(TID_D3DRMFrame, n.c_str(), 0, 0, 0, &root) );

		int nObjects = ig->GetTopLevelNodeCount();
		std::vector<IGameNode *> animated;
		for (int i = 0; i < nObjects; ++i) {
			IGameNode * n = ig->GetTopLevelNode(i);
			ExportNode(ig, n, save.p);
		}

		//  todo: write animations, if selected
		if (exportAnimation_) {
			ExportAnimations(save.p);
		}

		CHECK( save->Save() );
	}

	WNDPROC animationParentProc_;
	int selectedAnimation_ = -1;
	LRESULT CALLBACK AnimationWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_COMMAND || msg == WM_NOTIFY) {
			return ::SendMessage(::GetParent(hWnd), msg, wParam, lParam);
		}
		return CallWindowProc(animationParentProc_, hWnd, msg, wParam, lParam);
	}

	class MakeControlsVisitor {
	public:
		MakeControlsVisitor(HWND hWndDlg, IGameExporter * gex)
			: dlg_(hWndDlg)
			, gex_(gex)
		{
			LOGFONT lf;
			memset(&lf, 0, sizeof(lf));
			lf.lfHeight = rowHeight-4;
			lf.lfWidth = 0;
			lf.lfWeight = FW_REGULAR;
			lf.lfCharSet = ANSI_CHARSET;
			lf.lfOutPrecision = OUT_TT_PRECIS;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfQuality = 0; // ANTIALIASED_QUALITY | CLEARTYPE_QUALITY;
			lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
			strcpy(lf.lfFaceName, "Arial");
			font_ = ::CreateFontIndirect(&lf);
		}
		bool operator()(char const * group, bool & b, char const * name, bool v, char const * text, int id)
		{
			std::map<std::string, Info>::iterator ptr = groups_.find(group);
			Info i;
			if (ptr == groups_.end()) {
				i.frame = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
				if (i.frame == 0) {
					i.frame = dlg_;
				}
				i.row = 0;
				i.column = 0;
				groups_[group] = i;
				ptr = groups_.find(group);
			}
			else {
				i = (*ptr).second;
			}
			RECT r;
			::GetWindowRect(i.frame, &r);
			int left = 10;
			int top = 15;
			int colWidth = (r.right-r.left-15)/numCols;
			int numRows = (r.bottom-r.top-30)/rowHeight;
			HWND btn = ::CreateWindow(WC_BUTTON, text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 
				i.column * colWidth + left, i.row * rowHeight + top, 
				colWidth-5, rowHeight-2, i.frame, 0, hInstance, 0);
			//  I'm assuming the same string constant will always be used!
			SetWindowFont(btn, font_, false);
			SetWindowLong(btn, GWL_ID, WindowId(id));
			bool q = ::CheckDlgButton(i.frame, WindowId(id), b) != 0;
			i.row++;
			if (i.row >= numRows) {
				i.column++;
				i.row = 0;
			}
			(*ptr).second = i;
			return true;
		}
		bool operator()(char const * group, float & b, char const * name, float v, char const * text, int id)
		{
			std::map<std::string, Info>::iterator ptr = groups_.find(group);
			Info i;
			if (ptr == groups_.end()) {
				i.frame = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
				if (i.frame == 0) {
					i.frame = dlg_;
				}
				i.row = 0;
				i.column = 0;
				groups_[group] = i;
				ptr = groups_.find(group);
			}
			else {
				i = (*ptr).second;
			}
			RECT r;
			::GetWindowRect(i.frame, &r);
			int left = 10;
			int top = 15;
			int colWidth = (r.right-r.left-20)/numCols;
			int numRows = (r.bottom-r.top-30)/rowHeight;
			char val[20];
			_snprintf(val, 20, "%.3f", b);
			val[19] = 0;
			HWND lbl = ::CreateWindow(WC_STATIC, text, WS_CHILD | WS_VISIBLE,
				i.column * colWidth + left, i.row * rowHeight + top,
				labelWidth, rowHeight-2, i.frame, 0, hInstance, 0);
			HWND btn = ::CreateWindowEx(WS_EX_STATICEDGE,
				WC_EDIT, val, WS_CHILD | WS_VISIBLE, 
				i.column * colWidth + left+labelWidth, i.row * rowHeight + top, 
				colWidth-labelWidth-5, rowHeight-2, i.frame, 0, hInstance, 0);
			//  I'm assuming the same string constant will always be used!
			SetWindowFont(lbl, font_, false);
			SetWindowFont(btn, font_, false);
			::SetWindowLong(btn, GWL_ID, WindowId(id));
			i.row++;
			if (i.row >= numRows) {
				i.column++;
				i.row = 0;
			}
			(*ptr).second = i;
			return true;
		}
		bool operator()(char const * group, int & b, char const * name, int v, char const * text, int id)
		{
			std::map<std::string, Info>::iterator ptr = groups_.find(group);
			Info i;
			if (ptr == groups_.end()) {
				i.frame = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
				if (i.frame == 0) {
					i.frame = dlg_;
				}
				i.row = 0;
				i.column = 0;
				groups_[group] = i;
				ptr = groups_.find(group);
			}
			else {
				i = (*ptr).second;
			}
			RECT r;
			::GetWindowRect(i.frame, &r);
			int left = 10;
			int top = 15;
			int colWidth = (r.right-r.left-20)/numCols;
			int numRows = (r.bottom-r.top-30)/rowHeight;
			char val[20];
			if (!strcmp(name, "staticFrame") && b < 0) {
				b = gex_->coreFrame_;
			}
			_snprintf(val, 20, "%d", b);
			val[19] = 0;
			HWND lbl = ::CreateWindow(WC_STATIC, text, WS_CHILD | WS_VISIBLE,
				i.column * colWidth + left, i.row * rowHeight + top,
				labelWidth, rowHeight-2, i.frame, 0, hInstance, 0);
			HWND btn = ::CreateWindowEx(WS_EX_STATICEDGE,
				WC_EDIT, val, WS_CHILD | WS_VISIBLE, 
				i.column * colWidth + left+labelWidth, i.row * rowHeight + top, 
				colWidth-labelWidth-5, rowHeight-2, i.frame, 0, hInstance, 0);
			//  I'm assuming the same string constant will always be used!
			SetWindowFont(lbl, font_, false);
			SetWindowFont(btn, font_, false);
			::SetWindowLong(btn, GWL_ID, WindowId(id));
			i.row++;
			if (i.row >= numRows) {
				i.column++;
				i.row = 0;
			}
			(*ptr).second = i;
			return true;
		}
		bool operator()(char const * group, std::string & b, char const * name, std::string v, char const * text, int id)
		{
			std::map<std::string, Info>::iterator ptr = groups_.find(group);
			Info i;
			if (ptr == groups_.end()) {
				i.frame = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
				if (i.frame == 0) {
					i.frame = dlg_;
				}
				i.row = 0;
				i.column = 0;
				groups_[group] = i;
				ptr = groups_.find(group);
			}
			else {
				i = (*ptr).second;
			}
			RECT r;
			::GetWindowRect(i.frame, &r);
			int left = 10;
			int top = 15;
			int colWidth = (r.right-r.left-20)/numCols;
			int numRows = (r.bottom-r.top-30)/rowHeight;
			HWND btn = 0;
			if (!strcmp(name, ANIMATIONS_LIST_VIEW)) {
				btn = ::CreateWindow(WC_BUTTON, "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					i.column * colWidth + left, i.row * rowHeight + top,
					(colWidth-20)/3, rowHeight+1, i.frame, 0, hInstance, 0);
				SetWindowFont(btn, font_, false);
				::SetWindowLong(btn, GWL_ID, IDB_ADD);
				btn = ::CreateWindow(WC_BUTTON, "Update", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					i.column * colWidth + left + (colWidth-20)/3+10, i.row * rowHeight + top,
					(colWidth-20)/3, rowHeight+1, i.frame, 0, hInstance, 0);
				SetWindowFont(btn, font_, false);
				::SetWindowLong(btn, GWL_ID, IDB_UPDATE);
				btn = ::CreateWindow(WC_BUTTON, "Remove", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					i.column * colWidth + left + (colWidth-20)/3*2+20, i.row * rowHeight + top,
					(colWidth-20)/3, rowHeight+1, i.frame, 0, hInstance, 0);
				SetWindowFont(btn, font_, false);
				::SetWindowLong(btn, GWL_ID, IDB_REMOVE);
				++i.row;
				btn = ::CreateWindow(WC_LISTVIEW, "Animations", WS_CHILD | WS_VISIBLE | LVS_REPORT
					| LVS_EDITLABELS | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
					i.column * colWidth + left, i.row * rowHeight + top + 3,
					colWidth, rowHeight*6-5, i.frame, 0, hInstance, 0);
				ListView_SetExtendedListViewStyleEx(btn, 
					LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE,
					LVS_EX_FULLROWSELECT | LVS_EX_ONECLICKACTIVATE);
				i.row += 6;
				LVCOLUMN lvc;
				lvc.mask = LVCF_TEXT | LVCF_WIDTH;
				lvc.pszText = "Name";
				lvc.cx = colWidth-120;
				ListView_InsertColumn(btn, 0, &lvc);
				lvc.mask = LVCF_TEXT | LVCF_WIDTH;
				lvc.pszText = "Start";
				lvc.cx = 40;
				ListView_InsertColumn(btn, 1, &lvc);
				lvc.mask = LVCF_TEXT | LVCF_WIDTH;
				lvc.pszText = "Len";
				lvc.cx = 40;
				ListView_InsertColumn(btn, 2, &lvc);
				lvc.mask = LVCF_TEXT | LVCF_WIDTH;
				lvc.pszText = "Stretch";
				lvc.cx = 40;
				ListView_InsertColumn(btn, 3, &lvc);
				SetWindowFont(btn, font_, false);
				::SetWindowLong(btn, GWL_ID, IDC_ANIMATIONS);
				animationParentProc_ = (WNDPROC)::GetWindowLongPtr(i.frame, GWL_WNDPROC);
				::SetWindowLongPtr(i.frame, GWL_WNDPROC, (LONG)(LONG_PTR)&AnimationWndProc);
				std::vector<AnimationRange> vec;
				GetAnimationRanges(b, vec);
				LVITEM lvi;
				lvi.mask = LVIF_TEXT;
				char buf[512];
				buf[511] = 0;
				for (std::vector<AnimationRange>::iterator ptr = vec.begin(), end = vec.end();
					ptr != end; ++ptr) {
						lvi.iItem = ListView_GetItemCount(btn);
						lvi.iSubItem = 0;
						lvi.pszText = (LPSTR)(*ptr).name.c_str();
						lvi.iItem = ListView_InsertItem(btn, &lvi);
						sprintf(buf, "%d", (*ptr).first);
						lvi.pszText = buf;
						lvi.iSubItem = 1;
						ListView_SetItem(btn, &lvi);
						sprintf(buf, "%d", (*ptr).length);
						lvi.iSubItem = 2;
						ListView_SetItem(btn, &lvi);
						sprintf(buf, "%g", (*ptr).timeStretch);
						lvi.iSubItem = 3;
						ListView_SetItem(btn, &lvi);
				}
			}
			else {
				HWND lbl = ::CreateWindow(WC_STATIC, text, WS_CHILD | WS_VISIBLE,
					i.column * colWidth + left, i.row * rowHeight + top,
					labelWidth, rowHeight-2, i.frame, 0, hInstance, 0);
				btn = ::CreateWindowEx(WS_EX_STATICEDGE,
					WC_EDIT, b.c_str(), WS_CHILD | WS_VISIBLE, 
					i.column * colWidth + left+labelWidth, i.row * rowHeight + top, 
					colWidth-labelWidth-5, rowHeight-2, i.frame, 0, hInstance, 0);
				//  I'm assuming the same string constant will always be used!
				SetWindowFont(lbl, font_, false);
				SetWindowFont(btn, font_, false);
				::SetWindowLong(btn, GWL_ID, WindowId(id));
				i.row++;
			}
			if (i.row >= numRows) {
				i.column++;
				i.row = 0;
			}
			(*ptr).second = i;
			return true;
		}
		HWND dlg_;
		HFONT font_;
		struct Info {
			Info() { frame = 0; column = 0; row = 0; }
			HWND frame;
			int column;
			int row;
		};
		std::map<std::string, Info> groups_;
		IGameExporter * gex_;
	};

	class GetSettingsVisitor {
	public:
		HWND dlg_;
		int lastItem_;
		std::string lastItemName_;
		GetSettingsVisitor(HWND dlg) : dlg_(dlg) {}
		bool operator()(char const * group, bool & b, char const * name, bool v, char const * title, int id)
		{
			lastItem_ = WindowId(id);
			lastItemName_ = title;
			HWND t = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
			if (t == 0) t = dlg_;
			HWND i = ::GetDlgItem(t, WindowId(id));
			assert(i != 0);
			b = (::IsDlgButtonChecked(t, WindowId(id)) != 0);
			return true;
		}
		bool operator()(char const * group, float & b, char const * name, float v, char const * title, int id)
		{
			lastItem_ = WindowId(id);
			lastItemName_ = title;
			HWND t = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
			if (t == 0) t = dlg_;
			HWND i = ::GetDlgItem(t, WindowId(id));
			assert(i != 0);
			char text[512];
			::GetWindowText(i, text, 512);
			text[511] = 0;
			if (sscanf(text, "%f", &b) != 1) {
				return false;
			}
			//  there's a minimum and maximum value for floats
			if (b < 1e-3 || b > 1e3) {
				return false;
			}
			return true;
		}
		bool operator()(char const * group, int & b, char const * name, int v, char const * title, int id)
		{
			lastItem_ = WindowId(id);
			lastItemName_ = title;
			HWND t = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
			if (t == 0) t = dlg_;
			HWND i = ::GetDlgItem(t, WindowId(id));
			assert(i != 0);
			char text[512];
			::GetWindowText(i, text, 512);
			text[511] = 0;
			return sscanf(text, "%d", &b) == 1;
		}
		bool operator()(char const * group, std::string & b, char const * name, std::string v, char const * title, int id)
		{
			HWND t = ::FindWindowEx(dlg_, 0, WC_BUTTON, group);
			if (t == 0) t = dlg_;
			HWND i = ::GetDlgItem(t, WindowId(id));
			assert(i != 0);
			lastItem_ = WindowId(id);
			lastItemName_ = title;
			char text[512];
			text[511] = 0;
			if (!strcmp(name, ANIMATIONS_LIST_VIEW)) {
				int ni = ListView_GetItemCount(i);
				sprintf(text, "%d", ni);
				b = text;
				b += ";";
				for (int z = 0; z < ni; ++z) {
					ListView_GetItemText(i, z, 0, text, 511);
					b += convert_name(text);
					b += ",";
					ListView_GetItemText(i, z, 1, text, 511);
					b += text;
					b += ",";
					ListView_GetItemText(i, z, 2, text, 511);
					b += text;
					b += ",";
					ListView_GetItemText(i, z, 3, text, 511);
					b += text;
					b += ";";
				}
			}
			else {
				::GetWindowText(i, text, 511);
				b = text;
			}
			return true;
		}
	};

	INT_PTR CALLBACK SettingsProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IGameExporter * gex = (IGameExporter *)::GetWindowLongPtr(hWndDlg, GWL_USERDATA);
		HWND parent = ::FindWindowEx(hWndDlg, 0, WC_BUTTON, "Animation");
		HWND list = ::GetDlgItem(parent, IDC_ANIMATIONS);
		char buf[512];
		switch (msg) {
	case WM_INITDIALOG:
		gex = (IGameExporter *)lParam;
		::SetWindowLongPtr(hWndDlg, GWL_USERDATA, (LONG)lParam);
		Visit(*gex, MakeControlsVisitor(hWndDlg, gex));
		::SetWindowTextInt(::GetDlgItem(hWndDlg, IDC_FRAMERATE), GetFrameRate());
		::SetDlgItemText(hWndDlg, IDC_CREDITS, 
			"kW X-port by Jon Watte; built " __DATE__ 
			"\nhttp://kwxport.sourceforge.net/");
		return TRUE;
	case WM_CLOSE:
		::EndDialog(hWndDlg, 1);
		return TRUE;
	case WM_NOTIFY: {
		NMHDR const * nmh = (NMHDR const *)lParam;
		if (nmh->idFrom == IDC_ANIMATIONS) {
			switch (nmh->code) {
	case LVN_ITEMACTIVATE: {
		NMITEMACTIVATE const * nmi = (NMITEMACTIVATE const *)nmh;
		selectedAnimation_ = nmi->iItem;
		buf[511] = 0;
		ListView_GetItemText(list, selectedAnimation_, 0, buf, 511);
		::SetWindowText(::GetDlgItem(parent, IDC_ANIMATIONNAME), buf);
		ListView_GetItemText(list, selectedAnimation_, 1, buf, 511);
		::SetWindowText(::GetDlgItem(parent, IDC_FIRSTFRAME), buf);
		ListView_GetItemText(list, selectedAnimation_, 2, buf, 511);
		::SetWindowText(::GetDlgItem(parent, IDC_NUMFRAMES), buf);
		ListView_GetItemText(list, selectedAnimation_, 3, buf, 511);
		::SetWindowText(::GetDlgItem(parent, IDC_STRETCHTIME), buf);
						   }
						   break;
	default:
		break;
			}
		}
					}
					break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
	case IDB_ADD: {
		Settings s;
		if (!Visit(s, GetSettingsVisitor(hWndDlg))) {
			::MessageBox(hWndDlg, "Please enter valid data before continuing.", "kW X-port", MB_OK);
			return 0;
		}
		s.animationName_ = convert_name(s.animationName_.c_str());
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = 0;
		lvi.iSubItem = 0;
		lvi.pszText = (LPSTR)s.animationName_.c_str();
		HWND parent = ::FindWindowEx(hWndDlg, 0, WC_BUTTON, "Animation");
		HWND list = ::GetDlgItem(parent, IDC_ANIMATIONS);
		INT itm = (INT)::SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvi);
		if (itm >= 0) {
			char b[40];
			sprintf(b, "%d", s.firstFrame_);
			lvi.mask = LVIF_TEXT;
			lvi.iItem = itm;
			lvi.iSubItem = 1;
			lvi.pszText = b;
			::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
			sprintf(b, "%d", s.numFrames_);
			lvi.mask = LVIF_TEXT;
			lvi.iItem = itm;
			lvi.iSubItem = 2;
			lvi.pszText = b;
			::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
			sprintf(b, "%g", s.timeStretch_);
			lvi.mask = LVIF_TEXT;
			lvi.iItem = itm;
			lvi.iSubItem = 3;
			lvi.pszText = b;
			::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
		}
		selectedAnimation_ = ListView_GetSelectionMark(list);
				  }
				  break;
	case IDB_UPDATE: {
		selectedAnimation_ = ListView_GetSelectionMark(list);
		if (selectedAnimation_ < 0 || selectedAnimation_ >= ListView_GetItemCount(list)) {
			::MessageBox(hWndDlg, "Please select an animation first.", "kW X-port", MB_OK);
			return 0;
		}
		Settings s;
		if (!Visit(s, GetSettingsVisitor(hWndDlg))) {
			::MessageBox(hWndDlg, "Please enter valid data before continuing.", "kW X-port", MB_OK);
			return 0;
		}
		s.animationName_ = convert_name(s.animationName_.c_str());
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = selectedAnimation_;
		lvi.iSubItem = 0;
		lvi.pszText = (LPSTR)s.animationName_.c_str();
		HWND parent = ::FindWindowEx(hWndDlg, 0, WC_BUTTON, "Animation");
		HWND list = ::GetDlgItem(parent, IDC_ANIMATIONS);
		::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
		char b[40];
		sprintf(b, "%d", s.firstFrame_);
		lvi.mask = LVIF_TEXT;
		lvi.iItem = selectedAnimation_;
		lvi.iSubItem = 1;
		lvi.pszText = b;
		::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
		sprintf(b, "%d", s.numFrames_);
		lvi.mask = LVIF_TEXT;
		lvi.iItem = selectedAnimation_;
		lvi.iSubItem = 2;
		lvi.pszText = b;
		::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
		sprintf(b, "%g", s.timeStretch_);
		lvi.mask = LVIF_TEXT;
		lvi.iItem = selectedAnimation_;
		lvi.iSubItem = 3;
		lvi.pszText = b;
		::SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvi);
					 }
					 break;
	case IDB_REMOVE: {
		selectedAnimation_ = ListView_GetSelectionMark(list);
		if (selectedAnimation_ < 0 || selectedAnimation_ >= ListView_GetItemCount(list)) {
			::MessageBox(hWndDlg, "Please select an animation first.", "kW X-port", MB_OK);
			return 0;
		}
		ListView_DeleteItem(list, selectedAnimation_);
		selectedAnimation_ = -1;
					 }
					 break;
	case IDOK: {
		GetSettingsVisitor gsv(hWndDlg);
		if (!Visit(*gex, gsv)) {
			::MessageBox(hWndDlg, (std::string("The field '") + gsv.lastItemName_ + 
				"' contains an invalid value.").c_str(), "kW X-port", MB_OK);
		}
		else {
			::EndDialog(hWndDlg, 0);
		}
			   }
			   break;
	case IDCANCEL:
		::EndDialog(hWndDlg, 1);
		break;
		}
		return FALSE;
	default:
		return FALSE;
		}
		return FALSE;
	}

	//  This object doesn't actually support undo -- it just 
	//  implements the interface to be able to provoke the
	//  "file dirty" bit of the scene file, when settings 
	//  change during export.
	//  Some time in the future, perhaps real undo/redo would 
	//  be supported -- would only take two hours to implement.
	class NullRestoreObj : public RestoreObj {
	public:
		NullRestoreObj() {}
		virtual void Restore(int) {}
		virtual void Redo() {}
		virtual int Size() { return 10; }
	};

	int	IGameExporter::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
	{
		if (!dbgHelp) {
			dbgHelp = ::LoadLibrary("dbghelp.dll");
			if (dbgHelp != NULL) {
				*(void**)&WriteDumpFunc = (void *)::GetProcAddress(dbgHelp, "MiniDumpWriteDump");
			}
		}

		//  go from Max to DX
		globalMat.SetRow(0, Point4(1, 0, 0, 0));
		globalMat.SetRow(1, Point4(0, 0, -1, 0));
		globalMat.SetRow(2, Point4(0, 1, 0, 0));
		globalMat.SetRow(3, Point4(0, 0, 0, 1));

		int ret = 0;
		__try {
			ret = DoExport2(name, ei, i, suppressPrompts, options);
		}
		__except(MiniDumpHandler(GetExceptionInformation())) {
			if (!suppressPrompts) {
				::MessageBox(0, "kW X-port crashed while exporting.\nPlease save your work, revert to a back-up\n"
					"copy, and try again.\nWe apologize for the inconvenience.\n", "kW X-port", MB_OK);
			}
			else {
				DebugPrint("%s: Caught crash inside kW X-port.\n", name);
			}
		}
		return ret;
	}

	int	IGameExporter::DoExport2(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
	{
		if (!inited) {
			::InitCustomControls(hInstance);	// Initialize MAX's custom controls
			INITCOMMONCONTROLSEX iccx;
			iccx.dwSize = sizeof(iccx);
			iccx.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_USEREX_CLASSES | ICC_LISTVIEW_CLASSES;
			bool initControls = ::InitCommonControlsEx(&iccx) != 0;
			assert(initControls != false);
			inited = true;
		}

		time_t t = 0;
		time(&t);
		strftime(exportTime_, sizeof(exportTime_)/sizeof(exportTime_[0]), "%Y-%m-%d %H:%M:%S", localtime(&t));

		core_ = GetCOREInterface();
		name_ = name;
		MyErrorProc pErrorProc;
		SetErrorCallBack(&pErrorProc);

		bool exportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;
		float igameVersion  = GetIGameVersion();

		DebugPrint(_T("3ds max compatible version %.2f%\n"),GetSupported3DSVersion());

		igame_ = GetIGameInterface();

		IGameConversionManager * cm = GetConversionManager();
		cm->SetCoordSystem(IGameConversionManager::IGAME_MAX);

		if (staticFrame_ < 0) {
			coreFrame_ = core_->GetTime();
		}

		bool success = true;
		Settings old = *this;
		LoadSettings();
		if (!suppressPrompts) {
			if (::DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), 0, SettingsProc, 
				(LPARAM)this)) {
					//  restore previous settings
					*static_cast<Settings *>(this) = old;
					return true;
			}
		}

		coreFrame_ = staticFrame_;
		if (exportSelected) {
			Tab<INode *> nodetab;
			Interface * ip = GetCOREInterface();
			int snc = ip->GetSelNodeCount();
			for (int i = 0; i < snc; ++i) {
				INode *in = ip->GetSelNode(i);
				nodetab.Append(1, &in);
			}
			igame_->InitialiseIGame(nodetab, false);
		}
		else {
			igame_->InitialiseIGame(false);
		}
		igame_->SetStaticFrame(coreFrame_);

		try {
			theHold.Begin();
			SaveSettings();
			zMultiply = flipZ_ ? -1.0f : 1.0f;
			WriteXFile(igame_, name);
			theHold.Put(new NullRestoreObj());
			theHold.Accept(_T("Export Settings"));
		}
		catch(char const * error) {
#ifndef NDEBUG
			::OutputDebugString(error);
#endif
			theHold.Cancel();
			::MessageBox(0, error, "Error Writing X File", MB_OK);
			success = false;
		}
		catch(std::exception const &x) {
#ifndef NDEBUG
			::OutputDebugString(x.what());
#endif
			theHold.Cancel();
			::MessageBox(0, x.what(), "Error Writing X File", MB_OK);
			success = false;
		}

		igame_->ReleaseIGame();
		core_->ProgressEnd();	

		return success;
	}

	std::string IGameExporter::SettingsFilename()
	{
		TCHAR const * dir = core_->GetDir(APP_PLUGCFG_DIR);
		string s(dir);
		s += "\\kwxport.ini";
		return s;
	}

	class DocSettingsVisitor {
	public:
		std::map<string, string> & s_;
		DocSettingsVisitor(std::map<string, string> & s) : s_(s) {}
		bool operator()(char const * group, bool & b, char const * name, bool v, char const * title, int id)
		{
			std::map<string, string>::iterator ptr = s_.find(name);
			if (ptr != s_.end()) {
				int i = 0;
				if (1 == sscanf((*ptr).second.c_str(), "%d", &i)) {
					b = i != 0;
				}
			}
			return true;
		}
		bool operator()(char const * group, int & b, char const * name, int v, char const * title, int id)
		{
			std::map<string, string>::iterator ptr = s_.find(name);
			if (ptr != s_.end()) {
				int i = 0;
				if (1 == sscanf((*ptr).second.c_str(), "%d", &i)) {
					b = i;
				}
			}
			return true;
		}
		bool operator()(char const * group, float & b, char const * name, float v, char const * title, int id)
		{
			std::map<string, string>::iterator ptr = s_.find(name);
			if (ptr != s_.end()) {
				float i = 0;
				if (1 == sscanf((*ptr).second.c_str(), "%f", &i)) {
					b = i;
				}
			}
			return true;
		}
		bool operator()(char const * group, std::string & b, char const * name, std::string v, char const * title, int id)
		{
			std::map<string, string>::iterator ptr = s_.find(name);
			if (ptr != s_.end()) {
				b = (*ptr).second;
			}
			return true;
		}
	};

	//  Settings can live in one of three places:
	//  1) The defaults specified where the Visit() function is defined.
	//  2) The kwxport.ini plugin config settings file, which is updated 
	//    with the settings of whatever the last export is.
	//  3) The Max file itself, where settings are stored as file user props.
	//  The end result is that new files will inherit whatever the settings 
	//  were the last time an export was made, and previously exported files 
	//  will retain their export settings.
	//
	void IGameExporter::LoadSettings()
	{
		std::map<string, string> settings;
		FILE * f = fopen(SettingsFilename().c_str(), "rb");
		if (f) {
			char line[1024];
			while (true) {
				line[0] = 0;
				fgets(line, 1024, f);
				line[1023] = 0;
				if (!line[0]) break;
				char * x = strrchr(line, '\n');
				if (x) *x = 0;
				char * key = line;
				char * val = strchr(key, '=');
				if (val != 0) {
					*val = 0;
					++val;
					settings[key] = val;
				}
			}
			fclose(f);
		}
		int np = core_->GetNumProperties(PROPSET_USERDEFINED);
		for (int i = 0; i < np; ++i) {
			PROPSPEC const * ps = core_->GetPropertySpec(PROPSET_USERDEFINED, i);
			if (ps->ulKind == PRSPEC_LPWSTR) {
				PROPVARIANT const * pv = core_->GetPropertyVariant(PROPSET_USERDEFINED, i);
				string propName = wtocstr(ps->lpwstr);
				char str[1024];
				str[0] = 0;
				switch (pv->vt) {
	case VT_INT:
		sprintf(str, "%d", pv->intVal);
		break;
	case VT_I4:
		sprintf(str, "%d", pv->lVal);
		break;
	case VT_I8:
		sprintf(str, "%lld", pv->hVal);
		break;
	case VT_UINT:
		sprintf(str, "%u", pv->uintVal);
		break;
	case VT_UI4:
		sprintf(str, "%u", pv->ulVal);
		break;
	case VT_UI8:
		sprintf(str, "%llu", pv->uhVal);
		break;
	case VT_R4:
		sprintf(str, "%f", pv->fltVal);
		break;
	case VT_R8:
		sprintf(str, "%f", pv->dblVal);
		break;
	case VT_BOOL:
		sprintf(str, "%d", pv->boolVal ? 1 : 0);
		break;
	case VT_LPSTR:
		_snprintf(str, 1024, "%s", (char const *)pv->pszVal);
		str[1023] = 0;
		break;
	default:
		DebugPrint("Variant %s uses unknown variant type %d\n",
			propName.c_str(), pv->vt);
		break;
				}
				if (str[0]) {
					settings[propName] = str;
				}
			}
		}
		Visit(*this, DocSettingsVisitor(settings));
	}

	class SaveSettingsVisitor {
	public:
		SaveSettingsVisitor(FILE * f, Interface * core) : f_(f), core_(core) {}
		bool operator()(char const *, bool & b, char const * name, bool, char const *, int)
		{
			fprintf(f_, "%s=%d\n", name, b ? 1 : 0);
			PROPVARIANT pv;
			pv.vt = VT_BOOL;
			pv.boolVal = b;
			ReplaceProperty(name, pv);
			return true;
		}
		bool operator()(char const *, int & b, char const * name, int, char const *, int)
		{
			fprintf(f_, "%s=%d\n", name, b);
			PROPVARIANT pv;
			pv.vt = VT_I4;
			pv.lVal = b;
			ReplaceProperty(name, pv);
			return true;
		}
		bool operator()(char const *, float & b, char const * name, float, char const *, int)
		{
			fprintf(f_, "%s=%f\n", name, b);
			PROPVARIANT pv;
			pv.vt = VT_R4;
			pv.fltVal = b;
			ReplaceProperty(name, pv);
			return true;
		}
		bool operator()(char const *, std::string & b, char const * name, std::string, char const *, int)
		{
			fprintf(f_, "%s=%s\n", name, b.c_str());
			PROPVARIANT pv;
			pv.vt = VT_LPSTR;
			pv.pszVal = (LPSTR)b.c_str();
			ReplaceProperty(name, pv);
			return true;
		}
		void ReplaceProperty(char const * name, PROPVARIANT & pv)
		{
			PROPSPEC ps;
			wchar_t wstr[1024];
			mbstowcs(wstr, name, 1024);
			ps.ulKind = PRSPEC_LPWSTR;
			ps.lpwstr = wstr;
			core_->DeleteProperty(PROPSET_USERDEFINED, &ps);
			core_->AddProperty(PROPSET_USERDEFINED, &ps, &pv);
		}
		FILE * f_;
		Interface * core_;
	};

	void IGameExporter::SaveSettings()
	{
		FILE * f = fopen(SettingsFilename().c_str(), "wb");
		if (f) {
			Visit(*this, SaveSettingsVisitor(f, core_));
			fclose(f);
		}
		PROPSPEC ps;
		ps.ulKind = PRSPEC_LPWSTR;
		ps.lpwstr = L"lastExport";
		PROPVARIANT pv;
		pv.vt = VT_LPSTR;
		pv.pszVal = (LPSTR)exportTime_;
		core_->DeleteProperty(PROPSET_USERDEFINED, &ps);
		core_->AddProperty(PROPSET_USERDEFINED, &ps, &pv);
	}




	LRESULT __declspec(dllexport) __stdcall PluginCommit()
	{
		//  called when installer is done committing installation
		::ShellExecute(0, "open", "..\\d3dx_30\\DirectX End User EULA.txt", 0, 0, SW_SHOW);
		::ShellExecute(0, "open", "http://www.mindcontrol.org/~hplus/graphics/kwxport.html", 0, 0, SW_SHOW);
		return 0;
	}


	void assert_failure(char const *file, int line, char const *expr)
	{
		char buf[1024];
		_snprintf(buf, 1024, "%s:%d: Assertion failed\n%s\n", file, line, expr);
		buf[1023] = 0;
		throw std::exception(buf);
	}
