'use strict'

import { createRequire } from "module"
const {filelock} =  createRequire(import.meta.url)("./build/Release/filelock.node")

export default filelock;

/*
import {open, constants} from 'node:fs/promises';
async function testing()
{
    let fd = await open("/home/shahab/Desktop/testfile.txt", constants.O_CREAT | constants.O_RDWR, 0o600);
    let lock1 = ( await (new filelock(fd)).acquireWriteLock() );
    console.log(lock1);
    fd.close();
}

console.log("test1!");
testing();
console.log("test2!");
console.log("done!");*/