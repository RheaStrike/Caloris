var config = require('./config.js').getConfig();
var msgDef = require('./msg.js');

var fs = require('fs');
var http = require('http').createServer(
    function(req, res){
        fs.readFile('chat.html',function(err,data){
            res.writeHead(200, {'ContentType':'text/html'});
            res.write(data);
            res.end();
        });
    }
);

var socketio = require('socket.io').listen(http);

http.listen(config.port);

socketio.sockets.on('connection', function(socket){
    socket.username = 'undefined';

    socket.on('loginReq',function(data){
        var msg = JSON.parse(data);

        socket.username = msg.username;
        console.log(socket.id);

        var ack = msgDef.loginAck('true');
        socket.emit('loginAck', JSON.stringify(ack));
    });

    socket.on('chatReq',function(data){
        console.log(socket.id);
        if(!socket.username){
            return;
        }

        var msg = JSON.parse(data);

        var ack = msgDef.chatAck(socket.username, msg.chat);
        socketio.sockets.emit('chatAck', JSON.stringify(ack));
    });
});




