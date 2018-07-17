const fastify = require('fastify')();
const webpack = require('webpack');
const webpackDevMiddleware = require('webpack-dev-middleware');
const Handlebars = require('handlebars');
const config = require('./webpack.config.js');
const compiler = webpack(config);
const path = require('path');
const fs = require('fs');
const abs = file => path.resolve(__dirname, file);

fastify.register(require('fastify-static'), {
  root: path.join(__dirname, 'public'),
});

fastify.use(webpackDevMiddleware(compiler, {
  publicPath: config.output.publicPath
}));

fastify.use(require("webpack-hot-middleware")(compiler));

const indexTemplate = Handlebars.compile(fs.readFileSync(abs('./templates/index.handlebars'), 'utf8'))
fastify.get('/', (req, res) => {
  res.type('html');
  res.send(indexTemplate({
    CSSPATH: '/build/main.css',
    JSPATH: '/build/main.js'
  }));
});

const viewerTemplate = Handlebars.compile(fs.readFileSync(abs('./templates/viewer.handlebars'), 'utf8'))
fastify.get('/viewer/:id', (req, res) => {
  res.type('html');
  res.send(viewerTemplate({
    CSSPATH: '/build/viewer.css',
    JSPATH: '/build/viewer.js'
  }));
});

// Run the server!
const start = async () => {
  try {
    await fastify.listen(3005)
    fastify.log.info(`server listening on ${fastify.server.address().port}`)
  } catch (err) {
    console.log(err)
    fastify.log.error(err)
    process.exit(1)
  }
}
start()