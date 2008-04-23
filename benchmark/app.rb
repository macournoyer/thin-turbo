require 'rubygems'
require 'rack'
require 'rack/request'

class App
  MIN_SIZE = 512
  
  def initialize
    @calls = 0
  end
  
  def call(env)
    status  = 200
    body    = []
    request = Rack::Request.new(env)
    @calls += 1
    
    sleep request.params['wait'].to_i if request.params['wait'] && @calls % 10 == 0
    
    body   << 'X' * (request.params['size'] || MIN_SIZE).to_i
    body   << "\r\n"
    
    headers = { 'Content-Type' => 'text/plain', 'Content-Length' => body.join.size.to_s }
    
    [status, headers, body]
  end
end
