
exports.loginReq = function(name){
    var msg = {
        username : name
    };
    return msg;
}

exports.loginAck = function(result){
    var msg = {
        result : result
    };
    return msg;
}

exports.chatReq = function(msg){
    var msg = {
        chat :  msg
    };
    return msg;
}

exports.chatAck = function(name,msg){
    var msg = {
        from : name,
        chat : msg
    };
    return msg;
}
