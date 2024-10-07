from os.path import basename
from os import mkdir
import shutil
import gzip
    
Import("env")

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
# print(my_flags)
defines = {k: v for (k, v) in my_flags.get("CPPDEFINES")}
# print(defines)
bin_name = "wifi_sdcard_%s" % defines.get("APP_VERSION").replace('"', '')

def gzip_bin(source, target, env):
    firmware_path = str(source[0])
    firmware_name = basename(firmware_path)
    try:
	    mkdir('build')
    except:
      pass
    shutil.copy(firmware_path, f'build\{firmware_name}')
    with open(firmware_path, 'rb') as f_in, gzip.open(f'build\{firmware_name}.gz', 'wb') as f_out:
        shutil.copyfileobj(f_in, f_out)
    
env.Replace(PROGNAME=bin_name, UPLOADCMD=gzip_bin)

