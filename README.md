# Winmem
SlimIO Windmem is a NodeJS binding which expose low-level Microsoft APIs on Memory.

This binding expose the following methods/struct:

> !!! All method are called asynchronously without blocking the libuv event-loop !!!

## Getting Started

This package is available in the Node Package Repository and can be easily installed with [npm](https://docs.npmjs.com/getting-started/what-is-npm) or [yarn](https://yarnpkg.com).

```bash
$ npm i @slimio/winmem
# or
$ yarn add @slimio/winmem
```

## API

Media type enumeration can be retrieved [here](https://msdn.microsoft.com/en-us/library/windows/desktop/aa365231(v=vs.85).aspx).

## How to build the project

Before building the project, be sure to get the following npm package installed:

- Install (or upgrade to) NodeJS v10+ and npm v6+
- [Windows build tools](https://www.npmjs.com/package/windows-build-tools)

Then, execute these commands in order:

```bash
$ npm install
$ npx node-gyp configure
$ npx node-gyp build
```

## Available commands

All projects commands are described here:

| command | description |
| --- | --- |
| npm run prebuild | Generate addon prebuild |
| npm run doc | Generate JSDoc .HTML documentation (in the /docs root directory) |
| npm run coverage | Generate coverage of tests |
| npm run report | Generate .HTML report of tests coverage |

> the report command have to be triggered after the coverage command.
