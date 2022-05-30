const fs = require('fs');
const path = require('path');
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

const outDirPath = path.join(__dirname, argv[2]);
console.log("OutDir path: "+outDirPath);
fs.mkdir(outDirPath, { recursive: true },(err, path) => err ? console.log(err, path) : console.log("Path OK"));

for (i = 0; i < parseInt(argv[3]); ++i) {
    fs.writeFileSync(path.join(outDirPath, "sendme_" + i + ".txt"), makeid(100));
}


console.log("Files OK");
