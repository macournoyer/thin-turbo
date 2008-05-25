require 'yaml'
require 'rack/request'

class App
  attr_reader :env
  
  def call(env)
    @env = env
    @request = Rack::Request.new(env)
    body = process(env)
    [200, { 'Content-Type' => 'text/html', 'Content-Length' => Array(body).join.size.to_s }, body]
  end
  
  def params
    @request.params
  end
end

class DummyApp < App
  def process(env)
    ""    
  end
end

class EchoApp < App
  def process(env)
    if env['REQUEST_METHOD'] == 'GET'
      env['REQUEST_URI']
    else
      env['rack.input'].read
    end
  end
end

class StreamedApp < App
  def process(env)
    chunks = params['chunks'].to_i
    size   = params['size'].to_i
    (1..chunks).inject([]) { |body, i| body << 'X' * size }
  end
end