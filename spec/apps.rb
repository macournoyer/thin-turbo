require 'yaml'

class App
  attr_reader :env
  
  def call(env)
    @env = env
    body = process(env)
    [200, { 'Content-Type' => 'text/html', 'Content-Length' => body.size.to_s }, body]
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
