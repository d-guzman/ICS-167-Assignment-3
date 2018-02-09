var animate = window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || function (callback) {
        window.setTimeout(callback, 1000 / 60)
    };
// callback at ~60 calls/s or 60fps


var canvas = document.createElement('canvas');
var width = 700;
var height = 700;
canvas.width = width;
canvas.height = height;
var context = canvas.getContext('2d');

var player =  new Player(300, 580, 100, 10);
var ball = new Ball( 350, 350 );
var score = 0; //// HELP
var keysDown = {};

	
window.onload = function() {
	document.body.appendChild(canvas);
	animate(step);
};

window.addEventListener("keydown", function(event){
	keysDown[event.keyCode] = true;
});

window.addEventListener("keyup", function(event) {
	delete keysDown[event.keyCode];
	player.paddle.move(0,0);
});

var step = function(){
	update();
	render();
	animate(step);
};

//animating
var update = function() {
	player.update();
	ball.update(player.paddle);
};

// Play field
var render = function() {
	context.fillStyle = "#262626";
	context.fillRect( 100, 100, width-200, height-200 );
	player.render();
	ball.render();	
};

// Paddle(s)
function Paddle( x, y, width, height ){
	this.x = x;
	this.y = y;
	this.width = width;
	this.height = height;
	this.x_speed = 0;
	this.y_speed = 0;
}

Paddle.prototype.render = function() {
	context.fillStyle = "#f7f7f7";
	context.fillRect ( this.x, this.y, this.width, this.height );
};

Paddle.prototype.move = function( x, y ){
	this.x += x;
	this.y += y;
	this.x_speed = x;
	this.y_speed = y;
	
	if (this.x < 100 ){ // paddle all the way to the left
		this.x = 100;
		this.x_speed = 0;
	}
	else if ( this.x + this.width > 600) { // all the way to the right
		this.x = 600 - this.width;
		this.x_speed = 0;
	}
}

// Player
function Player(x, y, width, height){
	this.paddle = new Paddle( x, y, width, height ); // bottom paddle
}

Player.prototype.render = function() { 
	this.paddle.render()
};

Player.prototype.update = function() {
	for (var key in keysDown){
		var value = Number(key);
		if ( value == 37 ) { // left arrow
			this.paddle.move ( -4, 0 );	
		} 
		else if  (value == 39) {  // right arrow
			this.paddle.move( 4, 0 );
		}
		else {
			this.paddle.move( 0, 0 );
		}
	}
};

//Ball
function Ball( x, y ){
	this.x = x;
	this.y = y;
	this.x_speed = 0; 
	this.y_speed = 3;	// randomize these speeds later
	this.radius = 6;
}

Ball.prototype.render = function() {
	context.beginPath();
	context.arc( this.x, this.y, this.radius, 2 * Math.PI, false );
	context.fillStyle = "ffffff";
	context.fill();
}

Ball.prototype.update = function( paddle1 ){
	this.x += this.x_speed;
	this.y += this.y_speed;
	
	var top_x = this.x - 5;
	var top_y = this.y - 5;
	var bottom_x = this.x + 5;
	var bottom_y = this.y + 5;
	
	if (this.x - 5 < 100){ //hitting left wall
		this.x = 105;
		this.x_speed = -this.x_speed;
	} 
	else if ( this.x + 5 > 600 ) { // hitting right wall
		this.x = 595;
		this.x_speed = -this.x_speed;
	}
	else if  ( this.y - 5 < 100 ) { // hitting top wall
		this.y = 105;
		this.y_speed = -this.y_speed;
	}
	else if (this.y + 5 > 600){ // hitting bottom wall
		this.y = 595;
		this.y_speed = -this.y_speed;
		score = 0;
	}
	if(top_y > 300) {
	    if(top_y < (paddle1.y + paddle1.height) && bottom_y > paddle1.y && top_x < (paddle1.x + paddle1.width) && bottom_x > paddle1.x) {
	      // hit the player's paddle
			this.y_speed = -3;
		    this.x_speed += (paddle1.x_speed / 2);
		    this.y += this.y_speed;
			score++;
	    }
	}
};















