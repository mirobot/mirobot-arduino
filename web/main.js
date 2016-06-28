var MainMenu = function(el){
  var timer;
  
  var toggleMenu = function(e){
    el.classList.toggle('show');
    e.preventDefault();
    return false;
  }

  var hideMenu = function(e){
    el.classList.remove('show');
    if(e) e.preventDefault();
    return false;
  }  

  var handleKeyboard = function(e){
    if(e.keyCode === 27){
      hideMenu();
      e.preventDefault();
      return false;
    }
  }
  
  el.addEventListener('mouseup', toggleMenu);
  el.querySelector('.wrapper').addEventListener('mouseup', function(e){
    e.stopPropagation();
  });
  el.addEventListener('mouseleave', function(){
    timer = window.setTimeout(hideMenu, 500);
  });
  el.addEventListener('mouseenter', function(){
    if(timer){
      window.clearTimeout(timer);
      timer = undefined;
    }
  });
  window.addEventListener("keydown", function(e){ handleKeyboard(e);}, false);
}