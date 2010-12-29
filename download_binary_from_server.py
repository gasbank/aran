#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
2010 김거엽

외부 프로젝트에서 ARAN 라이브러리 바이너리 버젼을 바로 이용하고 싶은 경우에는
FTP 서버에 올려져 있는 것을 바로 다운로드할 수 있다.
본 파일은 해당 서버에 접속하여 이미 컴파일된 ARAN 라이브러리 바이너리 버젼을
받을 수 있도록 짠 스크립트이다.
현재에는 윈도우 전용으로 구현되어있는 상태.

파이썬이 실행되는 디렉토리에 aran 디렉토리를 생성하고
다운로드 받은 파일은 모두 이 디렉토리 안에 들어가도록 되어있다.
"""
import ftplib
import os
import sys

def traverse(ftp, depth=0):
  """
  return a recursive listing of an ftp server contents (starting
  from the current directory)

  listing is returned as a recursive dictionary, where each key
  contains a contents of the subdirectory or None if it corresponds
  to a file.

  @param ftp: ftplib.FTP object
  """
  if depth > 10:
      return ['depth > 10']
  level = {}
  for entry in (path for path in ftp.nlst() if path not in ('.', '..')):
      try:
          ftp.cwd(entry)
          level[entry] = traverse(ftp, depth+1)
          ftp.cwd('..')
      except ftplib.error_perm:
          level[entry] = None
  return level

def make_file_list(file_list, include_files, prefix):
  for k,v in include_files.iteritems():
    if v:
      make_file_list(file_list, v, prefix + k + '/')
    else:
      file_list.append(prefix + k)
      
def ftp_download_directory(ftp, dir_prefix, local_prefix):
  ftp.cwd(dir_prefix)
  include_files  =  traverse(ftp)
  file_list = []
  make_file_list(file_list, include_files, '')
  for f in file_list:
    remote_path = dir_prefix + f
    local_path = os.path.normpath(local_prefix + f)
    print dir_prefix + f, '-->', local_path
    try:
      os.makedirs(local_path[0: local_path.rfind('\\')])
    except:
      pass
    ftp.retrbinary('RETR ' + remote_path, open(local_path, 'wb').write)

def print_usage(argv):
  print 'You should specify two arguments.'
  print 'USAGE'
  print '  ', argv[0], '[vc80 | vc90 | vc10] [debug | release]'
  print
  print 'i.e.,', argv[0], 'vc90 debug'
  print
def main():
  if len(sys.argv) != 3 or sys.argv[1] not in ['vc80', 'vc90', 'vc10'] or sys.argv[2] not in ['debug', 'release']:
    print_usage(sys.argv)
    sys.exit(-2)
  vc_version = sys.argv[1]
  build_type = sys.argv[2]
  binary_server_addr = '143.248.139.103'
  login_id = 'aran'
  passphrase = 'gasbank_gmail.com'
  server_root = '/volume1/aran/'
  local_aran_root = './aran/'
  local_build_root = './build_%s/' % sys.argv[1]
  #---------------------------------------------
  print 'Please allow this program to use network resource if notified.'
  while True:
    answer = raw_input('  \'aran\' directory will be created in current directory.\n  Are you sure? (Y/n) ? ')
    if answer in ['Y', 'n']:
      break
    print 'Please answer Y or n.'
  if answer == 'Y':
    ftp = ftplib.FTP(binary_server_addr, login_id, passphrase)
    #            local_root  ,    server_rel_dir,                    local_rel_dir
    dirs = [ (local_aran_root,  'include/',                            None),
             (local_aran_root,  'lib/%s/%s/'%(vc_version,build_type),  None),
             (local_aran_root,  'resources/shaders/',                  None),
             (local_aran_root,  'resources/models/',                   None),
             (local_build_root, 'bin/%s/%s/'%(vc_version,build_type),  'bin/%s/'%build_type),
             (local_build_root, 'bin/thirdparties/'                 ,  'bin/%s/'%build_type),
             ]
    for d in dirs:
      if d[2]:
        ftp_download_directory(ftp, server_root + d[1], d[0] + d[2])
      else:
        ftp_download_directory(ftp, server_root + d[1], d[0] + d[1])
    print 'Finished.'
  else:
    print 'Abort.'

if __name__ == '__main__':
  main()
  