require 'rubygems'
require File.dirname(__FILE__) + '/../lib/thin-turbo'
require File.dirname(__FILE__) + '/apps'
require 'spec'
require 'thread'
require 'timeout'
require 'digest/sha1'

module ThinTurboServer
  ADDRESS = '0.0.0.0'
  PORT    = 7000
  
  def self.included(base)
    base.extend ClassMethods
  end
  
  module ClassMethods
    def serve(app=nil, &block)
      Thin::Logging.silent = true
      before do
        start_server(app || block)
      end
      after do
        stop_server
      end
    end
    
    def debug!
      Thin::Logging.silent = false
      Thin::Logging.debug  = true
      Thin::Logging.trace  = true
    end
  end
  
  def start_server(app)
    @app = app
    
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
    puts data if Thin::Logging.trace?
    socket = TCPSocket.new(ADDRESS, PORT)
    Array(data).each do |chunk|
      socket.write(chunk)
      socket.flush
    end
    out = nil
    Timeout.timeout(3) do
      out = socket.read
    end
  ensure
    socket.close
    puts out if Thin::Logging.trace?
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
      if @headers.has_key?(name)
        @headers[name] = [@headers[name]].flatten
        @headers[name] << value
      else
        @headers[name] = value
      end
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
    size = Array(body).join.size
    process_request 'POST', path, headers.merge('Content-Length' => size.to_s), body, &block
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

module Helpers
  def rand_data(min, max=min)
    count = min + ((rand(max)+1)).to_i
    (Digest::SHA1.hexdigest(rand(count * 100).to_s) * (count / 39))[0, max]
  end  
end

Spec::Runner.configure do |config|
  config.include ThinTurboServer
  config.include HttpSpecDSL
  config.include Helpers
end