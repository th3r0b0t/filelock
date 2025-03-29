'use strict'

import { createRequire } from "module"
const {filelock} =  createRequire(import.meta.url)("./build/Release/filelock.node")

export default filelock;

/*import {open, constants} from 'node:fs/promises';
async function testing()
{
    let fd = await open("/home/shahab/Desktop/testfile.txt", constants.O_CREAT | constants.O_RDWR, 0o600);
    let lock1 = new filelock(fd);
    console.log("lock1 Acquiring read lock...");
    console.log(await lock1.acquireReadLock());
    console.log("lock1 Acquired read lock!");

    let lock2 = new filelock(fd);
    console.log("lock2 Acquiring write lock...");
    console.log(await lock2.acquireWriteLock());
    console.log("lock2 Acquired write lock!");

    await lock1.removeLock();
    await fd.close();
    await lock2.removeLock();
}

console.log("test1!");
testing();
console.log("test2!");
console.log("done!");*/