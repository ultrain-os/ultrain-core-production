const webpack = require('webpack');
const path = require('path');

function resolve(dir) {
    return path.resolve(__dirname, dir);
}

module.exports = {
    entry: {
        app: ['babel-polyfill', './src/sideChainService.js'],
    },
    target: 'node',
    output: {
        path: __dirname,
        filename: './deploy/sideChainService.js'
    },
    resolve: {
        modules: [".", "node_modules"],
        extensions: ['.js'],
        alias: {
            "cfg": resolve("cfg.js")
        }
    },
    externals: function () {
        let manifest = require('./package.json');
        let dependencies = manifest.dependencies;
        let externals = {};
        for (let p in dependencies) {
            externals[p] = 'commonjs ' + p;
        }
        externals["cfg"] = "commonjs cfg";
        return externals;
    }(),
    node: {
        console: true,
        global: true,
        process: true,
        Buffer: true,
        __filename: true,
        __dirname: true,
        setImmediate: true
    },
    module: {
        rules: [
            {
                test: /\.js$/,
                loader: 'babel-loader',
                exclude: /node_modules/
            }
        ]
    }
};

if (process.env.NODE_ENV === 'production') {
    module.exports.plugins = (module.exports.plugins || []).concat([new webpack.optimize.UglifyJsPlugin({
        minimize: true,
        compress: false
    })]);
}

if (process.env.NODE_ENV === 'development') {
    module.exports.plugins = (module.exports.plugins || []).concat([new webpack.optimize.UglifyJsPlugin({
        minimize: false,
        compress: false
    })]);
}

