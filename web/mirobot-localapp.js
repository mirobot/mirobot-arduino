MirobotApp = function(options){
  options = options || {};
  window.l10n = (typeof options.l10n !== 'undefined' && options.l10n);
  this.simulation = !!options.simulation;
  this.languages =  options.languages;
  if(l10n) l10nMenu('l10n', this.languages);
  this.initFullScreenButton();
  this.mirobot = new Mirobot();
  this.mirobot.connect('ws://' + (window.hashConfig['m'] || window.location.hostname) + ':8899/websocket');
  if(this.simulation){
    var sim = new MirobotSim('sim', this.mirobot);
    this.mirobot.setSimulator(sim);
  }
  new MirobotConfigMenu(this.mirobot);
  new MirobotConnMenu(this.mirobot);
}

MirobotApp.prototype.supportsLocalStorage = function(){
  try {
    return 'localStorage' in window && window['localStorage'] !== null;
  } catch (e) {
    return false;
  }
}

MirobotApp.prototype.initPersistence = function(conf){
  if(this.supportsLocalStorage()){
    this.saveMenu = new MirobotSave(document.querySelector('#save'), conf);
  }
}

MirobotApp.prototype.initFullScreenButton = function(conf){
  if(typeof document.fullscreenEnabled === 'undefined') return document.querySelector('#fullscreen').classList.add('hidden');

  var setBodyClass = function(){
    var fn = document.fullscreenElement ? 'add' : 'remove'
    document.body.classList[fn]('fullscreen');
  }

  document.getElementById('fullscreen').addEventListener('click', function(){
    if (!document.fullscreenElement) {
      document.documentElement.requestFullscreen();
    } else {
      document.exitFullscreen();
    }
  });
  document.addEventListener('fullscreenchange', setBodyClass, false);
  setBodyClass();
}

