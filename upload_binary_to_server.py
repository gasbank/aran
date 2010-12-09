from ftplib import FTP
import os
import posixpath

def list_files_rec(files, path):
  subdirlist = []
  for fname in os.listdir(path):
    full_path = os.path.join(path, fname)
    if os.path.isfile(full_path):
      #print full_path, 'is file.'
      files.append(full_path)
    else:
      #print full_path, 'is directory.'
      subdirlist.append(full_path)
  for subdir in subdirlist:
    list_files_rec(files, subdir)

def upload_to_server(ftp, local_prefix, remote_prefix, local_files):
  for f in local_files:
    ftp_path = posixpath.normpath( f.replace(local_prefix, remote_prefix).replace('\\','/') ).lower()
    f = os.path.normpath(f)
    print f, '--->', ftp_path,
    try:
      for d in ftp_path.split('/')[:-1]:
        try:
          ftp.mkd(d)
        except:
          pass
        ftp.cwd(d)
      ftp.cwd('/')
      ftp.storbinary('STOR ' + ftp_path, open(f, 'rb'))
      print '...ok'
    except:
      print '...FAILED'

def get_files_end_with(path, suffix):
  files = []
  list_files_rec(files, path)
  files_filtered = []
  for f in files:
    if (suffix is None) or (suffix and f[f.rfind('.'):].lower() == suffix):
      files_filtered.append(os.path.normpath(f))
  return files_filtered

if __name__ == '__main__':
  binary_server_addr = '143.248.139.103'
  login_id = 'aran'
  passphrase = 'gasbank_gmail.com'
  #---------------------------------------------
  ftp = FTP(binary_server_addr, login_id, passphrase)
  work_path = os.environ['WORKING']
  codebase = os.environ['CODEBASE']
  
  dll_files = get_files_end_with(work_path + '/bin', '.dll')
  lib_files = get_files_end_with(work_path + '/lib', '.lib')
  shader_files = get_files_end_with(work_path + '/resources/shaders', '.vert')
  model_files = get_files_end_with(work_path + '/resources/models', '.xml')
  modelbin_files = get_files_end_with(work_path + '/resources/models', '.bin')
  h_files = get_files_end_with(codebase + '/arangit/src', '.h')
  inl_files = get_files_end_with(codebase + '/arangit/src', '.inl')
  
  upload_to_server(ftp, os.path.normpath(work_path + '/'), '/volume1/aran/', dll_files + lib_files)
  upload_to_server(ftp, os.path.normpath(work_path + '/'), '/volume1/aran/', shader_files)
  upload_to_server(ftp, os.path.normpath(work_path + '/'), '/volume1/aran/', model_files + modelbin_files)
  upload_to_server(ftp, os.path.normpath(codebase + '/arangit/src/'), '/volume1/aran/include/', h_files + inl_files)
  