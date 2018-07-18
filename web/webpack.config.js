const path = require('path');
const webpack = require('webpack')
const MiniCssExtractPlugin = require("mini-css-extract-plugin");

const abs = file => path.resolve(__dirname, file);
const isProduction = process.env.NODE_ENV === 'production';

module.exports = {
  entry: {
    main: [abs('./src/index.js')].concat(!isProduction ? ['webpack-hot-middleware/client'] : []),
    viewer: [abs('./src/viewer/index.js')].concat(!isProduction ? ['webpack-hot-middleware/client'] : []),
  },
  module: {
    rules: [
      {
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: ['babel-loader']
      },
      {
        test: /\.css$/,
        use: (isProduction ? [
          { loader: MiniCssExtractPlugin.loader },
        ] : [
          { loader: 'style-loader' },
        ]).concat([
          { loader: 'css-loader', options: { importLoaders: 1 } },
          { loader: 'postcss-loader', options: { plugins: [] } },
        ])
      }
    ]
  },
  plugins: isProduction ? [
    new MiniCssExtractPlugin({
      filename: "[name].css",
      chunkFilename: "[id].css"
    })
  ] : [
    new webpack.HotModuleReplacementPlugin(),
  ],
  resolve: {
    extensions: ['.js', '.jsx'],
  },
  mode: isProduction ? 'production' : 'development',
  devtool: isProduction ? false : 'cheap-eval-source-map',
  optimization: {
    splitChunks: {
      chunks: 'async',
      minSize: 30000,
      maxSize: 0,
      minChunks: 1,
      maxAsyncRequests: 5,
      maxInitialRequests: 3,
      automaticNameDelimiter: '~',
      name: true,
      cacheGroups: {
        reuseExistingChunk: false,
        vendor: {
          test: /[\\/]node_modules[\\/]/,
          priority: -10,
        reuseExistingChunk: false,
        },
        default: {
          minChunks: 2,
          priority: -20,
          reuseExistingChunk: false
        }
      }
    }
  },
  output: {
    path: abs('./public/build/'),
    publicPath: '/build/',
    filename: '[name].js'
  }
}