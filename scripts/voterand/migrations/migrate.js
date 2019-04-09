const {U3} = require('u3.js');
const config = require('../config');
const u3 = U3.createU3(config);
u3.deploy('build/MyContract', 'ben');