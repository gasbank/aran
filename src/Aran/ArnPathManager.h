#ifndef __ARNPATHMANAGER_H__
#define __ARNPATHMANAGER_H__
namespace aran {
  namespace core {
    class ARAN_API PathManager : public Singleton<PathManager>
    {
    public:
      void set_shader_dir(const char *c) { shader_path = c; }
      const std::string &get_shader_path() const { return shader_path; }
      void set_model_dir(const char *c) { model_path = c; }
      const std::string &get_model_path() const { return model_path; }
    private:
      std::string shader_path;
      std::string model_path;
    };
  } // namespace Core
} // namespace Aran

#endif // #ifndef __ARNPATHMANAGER_H__