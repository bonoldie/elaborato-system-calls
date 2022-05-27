const fs = require('fs');
const { argv } = require('process');

function makeid(length) {
    var result = '';
    var characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    var charactersLength = characters.length;
    for (var i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() *
            charactersLength));
    }
    return result;
}

for (i = 0; i < parseInt(argv[3]); ++i) {
    fs.writeFileSync(argv[2]+"/sendme_"+i+".txt",makeid(100));
}

