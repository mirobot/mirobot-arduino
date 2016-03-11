#! /usr/bin/env node

var fs = require("fs");
//var mime = require( "mime-types" );
var inline = require( "web-resource-inliner" );
var file =  "./web-raw/index.html";
var minify = require('html-minifier').minify;

var files = [
  {name: "index.html", process: true},
  {name: "favicon.ico", process: false}
]
var fileCount = files.length;

var baseDir = './web/';
var outFile = './src/lib/web.h';

var minifySettings = {minifyJS: true, minifyCSS: true}

outFiles = [];

function getMime(f){
  return "text/html";
}

function convert(f){
  var data = new Buffer(f[1]);
  byteData = []
  for(var i=0; i<data.length; i++){
    byteData.push("0x" + data[i].toString(16));
  }
  var dataname = f[0].replace('.', '_');
  return ["const char " + dataname + "[] PROGMEM = { " +  byteData + " };" , f[0], dataname, data.length, getMime(f[0])]
}

function makeHeaderFile(){
  if(fileCount) return;
  var data = outFiles.map(convert);
  output = data.map(function(d){
    return d[0];
  }).join("\n");
  output += "\nstruct t_websitefiles {\n  const char* filename;\n  const char* mime;\n  const unsigned int len;\n  const char* content;\n} files[] = {\n";
  output += data.map(function(d){
    return '  {.filename = "/' + d[1] + '", .mime = "' + d[4] + '", .len = ' + d[3] + ', .content = &' + d[2] + '[0]}'
  }).join(',\n');
  output += '};\nuint8_t fileCount = ' + files.length + ';\n';
  fs.writeFileSync(outFile, output);
}

files.map(function (f){
  if(f.process){
    // Process the file:
    // - bring all resources inline
    // - minify the css and js
    // - inline the images
    inline.html({
        fileContent: fs.readFileSync( baseDir + f.name, "utf8" ),
        strict: true,
        uglify: false,
        cssmin: false,
        scripts: true,
        relativeTo: baseDir
      },
      function( err, result ){
        if(err) return console.log(err);
        outFiles.push([f.name, minify(result, minifySettings)]);
        fileCount--;
        makeHeaderFile();
      }
    );
  }else{
    // Just copy the file across
    outFiles.push([f.name, fs.readFileSync( baseDir + f.name )]);
    fileCount--;
    makeHeaderFile();
  }
})
/*
import os
import mimetypes
*/

/*
def getMime(f):
  _, ext = os.path.splitext(f)
  return mimetypes.types_map[ext]
*/

/*
def convert(path, file):
  file = open(os.path.join(path, file))
  ba = bytearray(file.read())
  byteData = ", ".join("0x{:02x}".format(c) for c in ba)
  name = os.path.basename(file.name).replace('.', '_')
  return ("const char %s[] PROGMEM = { %s };" % (name, byteData), name, len(ba), getMime(file.name))
*/

/*
files = []
# Loop through the files in web-raw
for root, dirs, filenames in os.walk('./web-raw'):
  for f in filenames:
    if f[0] != '.':  
      print("Converting %s" % os.path.join(root, f))
      data, dataname, length, mime = convert(root, f);
      files.append({'data': data, 'name': '/' + os.path.basename(f), 'dataname': dataname, 'len': length, 'mime': mime})
*/
/*
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
*/
