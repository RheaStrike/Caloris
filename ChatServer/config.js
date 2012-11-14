function config(){
    this.port = 8080;
}

exports.getConfig = function(){
    return new config;
}