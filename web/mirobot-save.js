MirobotSave = function(el, conf){
  this.el = el;
  this.persister = new Persister(conf);
  this.init();
}

MirobotSave.prototype.createMenuItem = function(text, cb){
  var li = document.createElement('li');
  li.innerHTML = text;
  li.addEventListener('click', cb);
  return li
}

MirobotSave.prototype.createFileMenu = function(menu){
  var self = this;
  var progs_ul = menu.querySelector('ul#progs')
  if(!progs_ul){
    var progs_ul = document.createElement('ul');
    progs_ul.id = 'progs';
    progs_ul.className = 'subMenu';
    menu.appendChild(progs_ul);
  }else{
    progs_ul.innerHTML = '';
  }
  
  this.persister.fileList().map(function(f){
    progs_ul.appendChild(self.createMenuItem(f, function(){ self.openProgram(f);}));
  });
}

MirobotSave.prototype.setSaveFilename = function(name){
  var title = this.el.querySelector('#menu .title');
  if(name){
    title.innerHTML = '['+name+']';
  }else{
    title.innerHTML = '';
  }
}

MirobotSave.prototype.handleUpdate = function(){
  this.setSaveFilename(this.persister.currentProgram);
  this.createFileMenu(document.getElementById('save'));  
}

MirobotSave.prototype.init = function(){
  var self = this;
  this.persister.subscribe(function(){self.handleUpdate();});
  var wrap = document.createElement('div');
  wrap.className = 'wrapper';
  this.el.appendChild(wrap);
  var menu = document.createElement('ul');
  menu.id="saveMenu";
  menu.className="subMenu";
  menu.appendChild(this.createMenuItem(l(':save') + ' <span class="title"></span>', function(){ self.saveHandler();}));
  menu.appendChild(this.createMenuItem(l(':save-as') + '...', function(){ self.saveAsHandler();}));
  menu.appendChild(this.createMenuItem(l(':new-prog'), function(){ self.newHandler();}));
  menu.appendChild(this.createMenuItem(l(':delete-prog'), function(){ self.deleteHandler();}));
  menu.appendChild(this.createMenuItem(l(':download'), function(){ self.downloadHandler();}));
  var uploader = document.createElement('input');
  uploader.type = 'file';
  uploader.id = "uploader";
  wrap.appendChild(uploader);
  uploader.addEventListener('change', function(e){ self.uploadFileHandler(e) }, false);
  menu.appendChild(this.createMenuItem(l(':upload'), function(){ self.uploadHandler();}));
  
  var progs_li = document.createElement('li');
  progs_li.innerHTML = l(':open') + ':';
  progs_li.className = 'inactive';
  menu.appendChild(progs_li);
  wrap.appendChild(menu);
  if(this.persister.currentProgram){ this.setSaveFilename(this.persister.currentProgram);}
  
  this.createFileMenu(wrap);
  new MainMenu(this.el);
  this.el.classList.remove('hidden');

  window.addEventListener("keydown", function(e){ self.handleKeyboard(e);}, false);
}

MirobotSave.prototype.handleKeyboard = function(e){
  if(e.keyCode === 83 && e.metaKey){
    this.saveHandler();
    e.preventDefault();
    return false;
  }
}

MirobotSave.prototype.saveHandler = function(){
  if(this.persister.currentProgram){
    this.persister.save();
  }else{
    this.saveAsHandler();
  }
}

MirobotSave.prototype.saveAsHandler = function(){
  var filename = window.prompt(l(':choose-name'));
  if(filename && filename !== ''){
    if(this.persister.exists(filename)){
      alert(l(':exists'));
    }else{
      this.persister.saveAs(filename);
    }
  }
}

MirobotSave.prototype.uploadHandler = function(){
  if(this.checkSaved()){
    document.getElementById('uploader').click();
  }
}

MirobotSave.prototype.uploadFileHandler = function(e){
  var self = this;
  e.stopPropagation();
  e.preventDefault();
  if(typeof e.dataTransfer !== 'undefined'){
    var files = e.dataTransfer.files;
  }else if(typeof e.target !== 'undefined'){
    var files = e.target.files;
  }
  if(files.length > 1) return alert(l(':single-file'));
  
  // Read the file
  var r = new FileReader(files[0]);
  r.onload = function(e) { self.loadFromFile(e.target.result) }
  r.readAsText(files[0]);
  
  return false;
}

MirobotSave.prototype.loadFromFile = function(content){
  this.persister.new();
  this.persister.loadHandler(content);
}

MirobotSave.prototype.checkSaved = function(){
  if(this.persister.unsaved()){
    return window.confirm(l(':unsaved'))
  }
  return true;
}

MirobotSave.prototype.newHandler = function(){
  if(this.checkSaved()){
    this.persister.new();
  }
}

MirobotSave.prototype.downloadHandler = function(){
  this.persister.downloadCurrent();
}

MirobotSave.prototype.deleteHandler = function(){
  var filename = this.persister.currentProgram;
  if(filename && filename !== ''){
    if(confirm(l(':sure') + " '" + filename + "'? " + l(':permanent') + '.')){
      this.persister.delete(filename);
    }
  }
}

MirobotSave.prototype.openProgram = function(filename){
  if(this.checkSaved()){
    if(filename && filename !== '') this.persister.load(filename);
  }
}