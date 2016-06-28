var initLocalStorage = function(){
  try {
    localStorage.setItem('test', true);
    localStorage.removeItem('test');
    return window.localStorage;
  } catch (e) {
    // No local storage
    if('chrome' in window && 'storage' in window.chrome){
      // We are running as a chrome app
      return window.chrome.storage;
    }else{
      return false;
    }
  }
}

Line = function(x1, y1, x2, y2, steps, draw){
  this.start = {x:x1, y:y1}
  this.end = {x:x2, y:y2}
  this.totalSteps = steps;
  this.currentStep = 0;
  this.shouldDraw = draw;

  this.setEndpoint = function(x, y){
    this.end = {x:x, y:y}
  }

  this.draw = function(context, offset){
    if(this.shouldDraw){
      context.strokeStyle = '#000000';
      context.beginPath();
      context.lineWidth = 1;
      context.moveTo(this.start.x + offset.x, this.start.y + offset.y);
      context.lineTo(this.currentEndpoint().x + offset.x, this.currentEndpoint().y + offset.y);
      context.stroke();
      context.closePath();
    }
  }

  this.currentEndpoint = function(){
    if(this.currentStep > this.totalSteps){
      this.currentStep = this.totalSteps;
    }
    var dx = this.end.x - this.start.x;
    var dy = this.end.y - this.start.y;
    var _dx = (dx / this.totalSteps) * this.currentStep;
    var _dy = (dy / this.totalSteps) * this.currentStep;
    return {x: this.start.x + _dx, y: this.start.y + _dy};
  }
}

MirobotSim = function(button_id, mirobot){
  var self = this;
  this.localStorage = initLocalStorage();
  this.mirobot = mirobot;
  this.hide = false;
  this.button = document.getElementById(button_id);
  this.button.classList.remove('hidden');
  this.button.innerHTML = "<span>" + l(':simulate') + '</span> <div class="onoffswitch">\
    <input type="checkbox" name="onoffswitch" class="onoffswitch-checkbox" id="myonoffswitch">\
    <label class="onoffswitch-label" for="myonoffswitch"></label>\
</div>'
  var enable = this.button.querySelector('input');
  this.sim = document.createElement('div')
  this.sim.id = 'simArea';
  document.body.appendChild(this.sim);
  this.turtle = new Turtle(this.sim);
  this.resizer = document.createElement('div');
  this.resizer.id = 'resizer';
  this.resizer.innerHTML = '&#8690;';
  this.sim.appendChild(this.resizer);

  var label = document.createElement('span');
  label.className = 'label';
  label.innerHTML = l(":100mm-grid");
  this.sim.appendChild(label);

  var settings = document.createElement('div');
  settings.className = 'settings';
  settings.innerHTML = '<label>' + l(':fast-sim') + '<input type="checkbox" class="fastSim"></label><label>' + l(':hide-sim') + '<input type="checkbox" class="hideSim"></label>';

  // Setting to run faster
  var fastSim = settings.querySelector('.fastSim');
  fastSim.addEventListener('click', function(e){
    if(fastSim.checked){
      self.turtle.setSpeed(5);
    }else{
      self.turtle.setSpeed(1);
    }
  });

  // set the auto hide state
  var setHide = function(state){
    self.hide = state;
    hideSimEl.checked = state;
    if(self.localStorage) self.localStorage['mirobot-simulate-hide'] = state;
  }

  // Setting to auto hide the simulator
  var hideSimEl = settings.querySelector('.hideSim');
  hideSimEl.addEventListener('change', function(e){
    setHide(hideSimEl.checked);
    if(hideSimEl.checked){
      window.setTimeout(function(){
        self.sim.classList.remove('show');
      }, 500);
    }else{
    }
  });
  // Reinstate the localstorage setting for autohide
  if(this.localStorage && this.localStorage['mirobot-simulate-hide'] === 'true') setHide(true);

  // Add a reset button
  var reset = document.createElement('button');
  reset.innerHTML = l(":reset-sim");
  this.sim.appendChild(reset);
  reset.addEventListener('click', function(e){ self.turtle.reset(e) });

  this.sim.appendChild(settings);

  // Show the simulator
  var showSim = function(){
    self.sim.classList.add('show');
    self.turtle.init();
  }

  // Hide the simulator
  var hideSim = function(){
    self.sim.classList.remove('show');
  }

  // handle changes in the checkbox
  var inputChangeHandler = function(e){
    if(enable.checked){
      showSim();
    }else{
      setHide(false);
      hideSim();
    }
    if(e) e.preventDefault();
  }

  // Set simulation on or off
  var setSim = function(state, e){
    self.mirobot.setSimulating(state);
    enable.checked = state;
    inputChangeHandler(e);
    // store setting in localStorage
    if(self.localStorage) self.localStorage['mirobot-simulate'] = state;
  }
  if(this.localStorage && this.localStorage['mirobot-simulate'] === 'true') setSim(true);

  // Enable / disable the simulator
  enable.addEventListener('change', inputChangeHandler);

  // Handle the clicks fro the main button
  this.button.addEventListener('click', function(e){
    if(self.hide){
      // if autohide, just show or hide the sim
      self.sim.classList.contains('show') ? hideSim() : showSim();
    }else{
      // otherwise, turn simulation on or off
      setSim(!self.sim.classList.contains('show'), e);
    }
  });

  // show and auto hide the sim when clicking run
  this.mirobot.addEventListener('programStart', function(){
    if(self.mirobot.simulating){
      showSim();
      reset.disabled = true
    }
  });
  this.mirobot.addEventListener('programComplete', function(e){
    if(self.mirobot.simulating){
      reset.disabled = false
      if(self.hide){
        window.setTimeout(function(){
          self.sim.classList.remove('show');
        }, 3000);
      }
    }
  })

  if(self.localStorage && self.localStorage['mirobot-simulate-width']){
    self.sim.style.width = self.localStorage['mirobot-simulate-width'] + "px";
    self.sim.style.height = self.localStorage['mirobot-simulate-height'] + "px";
    self.turtle.resize();
  }

  var startX, startY, startW, startH;

  var drag = function(e){
    var diffX = e.clientX - startX;
    var diffY = e.clientY - startY;
    self.sim.style.width = startW - diffX + "px";
    self.sim.style.height = startH + diffY + "px";
    self.turtle.resize();
    e.preventDefault();
  }

  var startDrag = function(e){
    startX = e.clientX;
    startY = e.clientY;
    startW = self.sim.getBoundingClientRect().width;
    startH = self.sim.getBoundingClientRect().height;
    document.addEventListener('mousemove', drag, false);
    document.addEventListener('touchmove', drag, false);
    document.addEventListener('mouseup', endDrag, false);
    document.addEventListener('touchend', endDrag, false);
    e.preventDefault();
  }

  var endDrag = function(e){
    if(self.localStorage){
      self.localStorage['mirobot-simulate-width'] = self.sim.getBoundingClientRect().width;
      self.localStorage['mirobot-simulate-height'] = self.sim.getBoundingClientRect().height;
    }
    document.removeEventListener('mousemove', drag, false);
    document.removeEventListener('touchmove', drag, false);
    document.removeEventListener('mouseup', endDrag, false);
    document.removeEventListener('touchend', endDrag, false);
    e.preventDefault();
  }

  this.resizer.addEventListener('mousedown', startDrag, false);
  this.resizer.addEventListener('touchstart', startDrag, false);

  this.send = function(msg, cb){
    cb({status: 'accepted', id: msg.id});
    this.process(msg, cb)
  }

  var completeCb = function(cb, id, msg){
    var output = {status: 'complete', id: id};
    if(msg) output.msg = msg;
    return function(){
      cb(output);
    }
  }

  this.process = function(msg, cb){
    if(['stop', 'pause', 'resume', 'ping', 'version'].indexOf(msg.cmd) >= 0){
      if(msg.cmd === 'stop'){
        //stop the turtle moving
        this.turtle.stop(completeCb(cb, msg.id));
      }else if(msg.cmd === 'pause'){
        //stop the turtle moving
        this.turtle.pause(completeCb(cb, msg.id));
      }else if(msg.cmd === 'resume'){
        //stop the turtle moving
        this.turtle.resume(completeCb(cb, msg.id));
      }else if(msg.cmd === 'ping'){
        //stop the turtle moving
        completeCb(cb, msg.id)();
      }else if(msg.cmd === 'version'){
        //stop the turtle moving
        completeCb(cb, msg.id, 'sim')();
      }
    }else{
      if(self.turtle.moving){
        return cb({status: "error", id: msg.id});
      }
      if(msg.cmd === 'left'){
        var angle = -Number(msg.arg);
        this.turtle.rotate(angle, completeCb(cb, msg.id))
      }else if(msg.cmd === 'right'){
        var angle = Number(msg.arg);
        this.turtle.rotate(angle, completeCb(cb, msg.id))
      }else if(msg.cmd === 'forward'){
        var distance = Number(msg.arg);
        this.turtle.move(distance, completeCb(cb, msg.id))
      }else if(msg.cmd === 'back'){
        var distance = -Number(msg.arg);
        this.turtle.move(distance, completeCb(cb, msg.id))
      }else if(msg.cmd === 'penup'){
        this.turtle.penup(completeCb(cb, msg.id))
      }else if(msg.cmd === 'pendown'){
        this.turtle.pendown(completeCb(cb, msg.id))
      }else if(msg.cmd === 'beep'){
        this.turtle.beep(Number(msg.arg), completeCb(cb, msg.id))
      }else{
        return cb({status: "error", id: msg.id});
      }
    }
  }
}


var Turtle = function(el){
  var self = this;
  this.el = el;
  this.penDown = true;
  this.moving = false;
  this.prev_angle = 0;
  this.curr_angle = 0;
  this.angle = 0;
  this.robotLoc = {x:0, y:0};
  this.drawList = [];
  this.initted = false;
  this.speed = 1;
  this.stopped = false;

  function rotatePoint(point, angle){
    var xr = point[0] * Math.cos(angle) - point[1] * Math.sin(angle);
    var yr = point[0] * Math.sin(angle) + point[1] * Math.cos(angle);
    return [xr, yr];
  }
  function rotate(points, angle) {
    var angle = angle * Math.PI / 180 // Convert to radians
    for(var i=0; i<points.length; i++){
      points[i] = rotatePoint(points[i], angle);
    }
  }

  this.setSpeed = function(speed){
    this.speed = speed;
  }

  this.remapX = function(x){
    return x + this.canvas.getBoundingClientRect().width/2;
  }

  this.remapY = function(y){
    return y + this.canvas.getBoundingClientRect().height/2;
  }

  this.drawTurtle = function(){
    var points = [[6, 6], [0, -10], [-6, 6]]
    rotate(points, this.angle);

    this.context.strokeStyle = '#EE0000';
    this.context.fillStyle = (this.penDown ? "rgba(255, 0, 0, 0.7)" : "rgba(255, 255, 255, 0.7)");
    this.context.beginPath();
    this.context.lineWidth = 1;
    this.context.moveTo(self.remapX(this.robotLoc.x+points[2][0]), self.remapY(this.robotLoc.y+points[2][1]));
    for(var i = 0; i<points.length; i++){
      this.context.lineTo(self.remapX(this.robotLoc.x+points[i][0]), self.remapY(this.robotLoc.y+points[i][1]));
    }
    this.context.stroke();
    this.context.fill();
    this.context.closePath();
  }

  this.draw = function(){
    //clear the canvas
    this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
    //draw the base grid
    this.drawGrid();
    //draw each line
    for(var i=0; i<self.drawList.length; i++){
      self.drawList[i].draw(self.context, {x: self.remapX(0), y: self.remapY(0)});
    }
    //draw the robot
    this.drawTurtle();
  }

  this.move = function(distance, cb){
    if(this.moving) return;
    this.moving = true;
    this.stopped = false;
    var rads = this.angle * (Math.PI/180);
    if(distance < 0) rads -= Math.PI;
    distance = Math.abs(distance);
    var rate = 1;
    var destX = this.robotLoc.x + Math.sin(rads) * distance;
    var destY = this.robotLoc.y - Math.cos(rads) * distance;
    var steps = Math.ceil(distance/rate);

    this.drawList.push(new Line(this.robotLoc.x, this.robotLoc.y, destX, destY, steps, this.penDown));

    var animate = function(){
      if(self.stopped) return;
      if(self.paused) return window.requestAnimationFrame(animate);
      var lastLine = self.drawList[self.drawList.length - 1];
      if(lastLine.currentStep < lastLine.totalSteps){
        lastLine.currentStep += self.speed;
        self.robotLoc.x = lastLine.currentEndpoint().x;
        self.robotLoc.y = lastLine.currentEndpoint().y;
        self.draw();
        window.requestAnimationFrame(animate);
      }else{
        self.moving = false;
        cb();
      }
    }
    animate();
  }

  this.rotate = function(angle, cb){
    if(this.moving) return;
    this.moving = true;
    this.stopped = false;
    var rate = 1;
    var steps = Math.abs(angle / rate);
    var amount = angle / steps;
    var startAngle = self.angle;
    var step = 0;

    var animate = function(){
      if(self.stopped) return;
      if(self.paused) return window.requestAnimationFrame(animate);
      step += self.speed;
      if(step > steps) step = steps;
      self.angle = startAngle + (amount * step);
      self.draw();
      if(step === steps){
        self.moving = false;
        cb();
      }else{
        window.requestAnimationFrame(animate);
      }
    }
    animate();
  }

  this.penup = function(cb){
    this.penDown = false;
    this.draw();
    window.setTimeout(cb, 250);
  }

  this.pendown = function(cb){
    this.penDown = true;
    this.draw();
    window.setTimeout(cb, 250);
  }

  this.beep = function(duration, cb){
    //TODO: use sound in the browser
    window.setTimeout(cb, duration);
  }

  this.stop = function(cb){
    this.stopped = true;
    this.moving = false;
    if(cb) cb();
  }

  this.pause = function(cb){
    this.paused = true;
    cb();
  }

  this.resume = function(cb){
    this.paused = false;
    cb();
  }

  this.reset = function(e){
    this.moving = false;
    this.angle = 0;
    this.robotLoc={x:0,y:0};
    this.context.clearRect(0, 0, this.canvas.width, this.canvas.height);
    this.drawList = [];
    this.stop();
    this.draw();
    e.preventDefault();
    e.cancelBubble = true;
  }

  this.drawGrid = function(){
    var gridSize = 100;
    var w = this.canvas.getBoundingClientRect().width;
    var h = this.canvas.getBoundingClientRect().height;
    var x = (w/2)%gridSize;
    var y = (h/2)%gridSize;
    this.context.beginPath();
    this.context.strokeStyle = '#DDD';
    this.context.lineWidth = 1;
    while(y < h){
      this.context.moveTo(0, y);
      this.context.lineTo(w, y);
      this.context.stroke();
      y += gridSize;
    }

    while(x < w){
      this.context.moveTo(x, 0);
      this.context.lineTo(x, h);
      this.context.stroke();
      x += gridSize;
    }
    this.context.closePath();
  }

  this.resize = function(){
    this.canvas.width = this.el.clientWidth;
    this.canvas.height = this.el.clientHeight;

    this.context.canvas.width = this.canvas.getBoundingClientRect().width;
    this.context.canvas.height = this.canvas.getBoundingClientRect().height;
    this.draw();
  }

  this.init = function(){
    if(!this.initted){
      this.initted = true;
      this.resize();
      this.draw();
    }
  }

  this.addTurtle = function(){
    this.canvas = document.createElement('canvas');
    this.context = this.canvas.getContext('2d');
    this.el.appendChild(this.canvas);
  }

  this.addTurtle();
}
