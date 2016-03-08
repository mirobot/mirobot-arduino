#! /usr/bin/env python
import os
import mimetypes

def getMime(f):
  _, ext = os.path.splitext(f)
  return mimetypes.types_map[ext]

def convert(path, file):
  file = open(os.path.join(path, file))
  ba = bytearray(file.read())
  byteData = ", ".join("0x{:02x}".format(c) for c in ba)
  name = os.path.basename(file.name).replace('.', '_')
  return ("const char %s[] PROGMEM = { %s };" % (name, byteData), name, len(ba), getMime(file.name))

files = []
# Loop through the files in web-raw
for root, dirs, filenames in os.walk('./web-raw'):
  for f in filenames:
    if f[0] != '.':  
      print("Converting %s" % os.path.join(root, f))
      data, dataname, length, mime = convert(root, f);
      files.append({'data': data, 'name': '/' + os.path.basename(f), 'dataname': dataname, 'len': length, 'mime': mime})

output = "\n".join(map(lambda f: f['data'], files))
  
output += """
struct t_websitefiles {
  const char* filename;
  const char* mime;
  const unsigned int len;
  const char* content;
} files[] = {"""


output += ",\n".join(map(lambda f: '{.filename = "%s", .mime = "%s", .len = %d, .content = &%s[0]}' % (f['name'], f['mime'], f['len'], f['dataname']), files))
output += "};\nuint8_t fileCount = %d;\n" % len(files)


outfile = open('./src/lib/web_pages.h', 'w')
outfile.write(output)
outfile.close()
