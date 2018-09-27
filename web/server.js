const fastify = require('fastify')();
const webpack = require('webpack');
const webpackDevMiddleware = require('webpack-dev-middleware');
const Handlebars = require('handlebars');
const config = require('./webpack.config.js');
const compiler = webpack(config);
const path = require('path');
const fs = require('fs');

const abs = file => path.resolve(__dirname, file);
const isProduction = process.env.NODE_ENV === 'production';

fastify.register(require('fastify-static'), {
  root: path.join(__dirname, 'public'),
});

fastify.use(webpackDevMiddleware(compiler, {
  publicPath: config.output.publicPath,
}));

fastify.use(require("webpack-hot-middleware")(compiler));

fastify.get('/', (req, res) => {
  const indexTemplate = Handlebars.compile(fs.readFileSync(abs('./templates/index.handlebars'), 'utf8'));
  res.type('html');
  res.send(indexTemplate({
    CSSPATH: isProduction ? '/build/main.css' : false,
    JSPATH: '/build/main.js',
  }));
});

fastify.get('/viewer/:id', (req, res) => {
  const viewerTemplate = Handlebars.compile(fs.readFileSync(abs('./templates/viewer.handlebars'), 'utf8'));
  res.type('html');
  res.send(viewerTemplate({
    CSSPATH: '/build/viewer.css',
    JSPATH: '/build/viewer.js',
  }));
});

// Run the server!
const start = async () => {
  try {
    await fastify.listen(3005, '0.0.0.0');
    fastify.log.info(`server listening on ${fastify.server.address().port}`);
  } catch (err) {
    console.log(err);
    fastify.log.error(err);
    process.exit(1);
  }
};
start();
