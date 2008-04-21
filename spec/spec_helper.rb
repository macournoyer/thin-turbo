require 'rubygems'
require File.dirname(__FILE__) + '/../lib/thin-turbo'
require File.dirname(__FILE__) + '/apps'
require 'spec'
require 'thread'
require 'timeout'

module ThinTurboServer
  ADDRESS = '0.0.0.0'
  PORT    = 7000
  
  def self.included(base)
    base.extend ClassMethods
  end
  
  module ClassMethods
    def serve(app=nil, &block)
      before do
        start_server(app || block)
      end
      after do
        stop_server
      end
    end    
  end
  
  def start_server(app)
    @app = app
    
    Thin::Logging.silent = true
    @server = Thin::Server.new(ADDRESS, PORT, @app, :backend => Thin::Backends::Turbo, :signals => false)
    
    @thread = Thread.new { @server.start }
    
    wait_for { ping_server }
  end
  
  def ping_server
    begin
      TCPSocket.new(ADDRESS, PORT).close
      true
    rescue
      false
    end
  end
  
  def stop_server
    @server.stop!
    wait_for { !ping_server }
    @thread.kill
  end
  
  def wait_for(sec=3)
    Timeout.timeout(sec) do
      loop do
        return if yield
        Thread.pass
      end
    end
  end
  
  def request(data)
    socket = TCPSocket.new(ADDRESS, PORT)
    socket.write(data)
    socket.flush
    out = nil
    Timeout.timeout(3) do
      out = socket.read
    end
  ensure
    socket.close
    out
  end
end

class HttpResponse
  attr_reader :status, :headers, :body
  
  def initialize(data)
    @data = data
    
    if matches = data.match(/^HTTP\/\d\.\d (\d{3}) [\w\s]+\r$/)
      @status = matches[1].to_i
    end
    
    @headers = {}
    data.scan(/^([^\(|\)\<\>@,;:\\\"\/\[\]\?=\{\}\s\t]+): (.*)\r$/).each do |name, value|
      @headers[name] = value
    end
    
    @body = data.split("\r\n\r\n")[1..-1].join("\r\n\r\n") if data.include?("\r\n\r\n")
  end
  
  def to_s
    @data
  end
end

module HttpSpecDSL
  DEFAULT_HEADERS = { 'Connection' => 'close', 'Host' => 'localhost' }
  
  def GET(path, headers=DEFAULT_HEADERS, &block)
    process_request 'GET', path, headers, &block
  end
  
  def POST(path, body, headers=DEFAULT_HEADERS, &block)
    process_request 'POST', path, headers, body, &block
  end
  
  private
    def process_request(method, path, headers, body=nil, &block)
      unless @_defined_description
        @_defined_description = "should #{method} #{path} #{headers.inspect}"
        @_defined_description << " " + body.to_s.split("\r\n").first if body
      end
      
      h = headers.inject('') { |h, (k,v)| h += "#{k}: #{v}\r\n" }
      response = HttpResponse.new(request("#{method} #{path} HTTP/1.1\r\n#{h}\r\n#{body}"))
      
      response.instance_eval(&block) if block
      response
    end
end

Spec::Runner.configure do |config|
  config.include ThinTurboServer
  config.include HttpSpecDSL
end