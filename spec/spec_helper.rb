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
    def serve(app)
      before do
        start_server app
      end
      after do
        stop_server
      end
    end    
  end
  
  def start_server(app)
    Thin::Logging.silent = true
    @server = Thin::Server.new(ADDRESS, PORT, app, :backend => Thin::Backends::Turbo, :signals => false)
    
    @thread.kill if @thread
    @thread = Thread.new { @server.start }
    
    Timeout.timeout(3) do
      loop do
        return if ping_server
        Thread.pass
      end
    end
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
    @server.stop
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

Spec::Runner.configure do |config|
  config.include ThinTurboServer
end