/**
 * This script bootstraps vcpkg and uses it to install the required package port scripts
 */
const { execFileSync, spawn } = require('child_process');
const { existsSync } = require('fs');
const { join } = require('path');

var args = ['install'];

var options = {
    stdio: 'inherit', //feed all child process logging into parent process
    shell: true,
    cwd: join(process.cwd(),"vcpkg")
};

const vcpkgBinary = (process.platform === 'win32') ?
    "vcpkg\\vcpkg.exe" : "vcpkg/vcpkg";

function spawnBootstrap() {
    if (existsSync(vcpkgBinary)) {
        console.log("found " + vcpkgBinary + " .. starting PortInstall")
        spawnPortInstall();
        return;
    }
    else {
        console.log("did NOT find " + vcpkgBinary + " .. starting PortInstall")
        const bootStrapScript = (process.platform === 'win32') ?
            "bootstrap-vcpkg.bat" :
            "bootstrap-vcpkg.sh";

        console.log(options);
        var bootstrapProcess = spawn(join(options.cwd, bootStrapScript), options);
        bootstrapProcess.on('close', function (code) {
            process.stdout.write('"bootstrap" finished with code ' + code + '\n');

            if (code != 0) {
                process.exit(code);
            }

            spawnPortInstall();
        });
    }
}

function spawnPortInstall() {
    const portInstallResponsefile = (process.platform == 'win32') ?
        "@..\\vcpkg_x64-windows.txt" :
        (process.platform == 'darwin') ?
            "@../vcpkg_x64-osx.txt" :
            "@../vcpkg_x64-linux.txt"

    var portInstallProces = spawn(join(process.cwd(),vcpkgBinary), ["install", portInstallResponsefile], options)

    portInstallProces.on('close', function (code) {
        process.stdout.write('"bootstrap" finished with code ' + code + '\n');
        process.exit(code);
    });
}

spawnBootstrap();