require 'rubygems'
require File.dirname(__FILE__) + '/../lib/thin'
require 'spec'
require 'benchmark'
require 'timeout'
require 'fileutils'
require 'benchmark_unit'
require 'net/http'
require 'socket'

include Thin

FileUtils.mkdir_p File.dirname(__FILE__) + '/../log'
Command.script = File.dirname(__FILE__) + '/../bin/thin'
Logging.silent = true

SWIFTIPLY_PATH = `which swiftiply`.chomp unless Object.const_defined?(:SWIFTIPLY_PATH)

module Matchers
  class BeFasterThen
    def initialize(max_time)
      @max_time = max_time
    end

    # Base on benchmark_unit/assertions#compare_benchmarks
    def matches?(proc)
      @time, multiplier = 0, 1
      
      while (@time < 0.01) do
        @time = Benchmark::Unit.measure do 
          multiplier.times &proc
        end
        multiplier *= 10
      end
      
      multiplier /= 10
      
      iterations = (Benchmark::Unit::CLOCK_TARGET / @time).to_i * multiplier
      iterations = 1 if iterations < 1
      
      total = Benchmark::Unit.measure do 
        iterations.times &proc
      end
      
      @time = total / iterations
      
      @time < @max_time
    end
    
    def failure_message(less_more=:less)
      "took <#{@time.inspect} RubySeconds>, should take #{less_more} than #{@max_time} RubySeconds."
    end

    def negative_failure_message
      failure_message :more
    end
  end
  
  class ValidateWithLint
    def matches?(request)
      @request = request
      Rack::Lint.new(proc{[200, {'Content-Type' => 'text/html'}, []]}).call(@request.env)
      true
    rescue Rack::Lint::LintError => e
      @message = e.message
      false
    end
    
    def failure_message(negation=nil)
      "should#{negation} validate with Rack Lint"
    end

    def negative_failure_message
      failure_message ' not'
    end
  end

  class TakeLessThen
    def initialize(time)
      @time = time
    end
    
    def matches?(proc)
      Timeout.timeout(@time) { proc.call }
      true
    rescue Timeout::Error
      false 
    end
    
    def failure_message(negation=nil)
      "should#{negation} take less then #{@time} sec to run"
    end

    def negative_failure_message
      failure_message ' not'
    end
  end

  # Actual matchers that are exposed.

  def be_faster_then(time)
    BeFasterThen.new(time)
  end
  
  def validate_with_lint
    ValidateWithLint.new
  end

  def take_less_then(time)
    TakeLessThen.new(time)
  end  
end

module Helpers
  # Silences any stream for the duration of the block.
  #
  #   silence_stream(STDOUT) do
  #     puts 'This will never be seen'
  #   end
  #
  #   puts 'But this will'
  #
  # (Taken from ActiveSupport)
  def silence_stream(stream)
    old_stream = stream.dup
    stream.reopen(RUBY_PLATFORM =~ /mswin/ ? 'NUL:' : '/dev/null')
    stream.sync = true
    yield
  ensure
    stream.reopen(old_stream)
  end
  
  # Create and parse a request
  def R(raw, convert_line_feed=false)
    raw.gsub!("\n", "\r\n") if convert_line_feed
    request = Thin::Request.new
    request.parse(raw)
    request
  end
  
  def start_server(address='0.0.0.0', port=3333, wait_for_socket=true, &app)
    @server = Thin::Server.new(address, port, app)
    @server.timeout = 3
    
    @thread = Thread.new { @server.start }
    if wait_for_socket
      wait_for_socket(address, port)
    else
      # If we can't ping the address fallback to just wait for the server to run
      sleep 1 until @server.running?
    end
  end
  
  def stop_server
    @server.stop!
    @thread.kill
    raise "Reactor still running, wtf?" if EventMachine.reactor_running?
  end
  
  def wait_for_socket(address='0.0.0.0', port=3333, timeout=5)
    Timeout.timeout(timeout) do
      loop do
        begin
          if address.include?('/')
            UNIXSocket.new(address).close
          else
            TCPSocket.new(address, port).close
          end
          return true
        rescue
        end
      end
    end
  end
    
  def send_data(data)
    if @server.backend.class == Backends::UnixServer
      socket = UNIXSocket.new(@server.socket)
    else
      socket = TCPSocket.new(@server.host, @server.port)
    end
    socket.write data
    out = socket.read
    socket.close
    out
  end
  
  def get(url)
    if @server.backend.class == Backends::UnixServer
      send_data("GET #{url} HTTP/1.1\r\nConnection: close\r\n\r\n")
    else
      Net::HTTP.get(URI.parse("http://#{@server.host}:#{@server.port}" + url))
    end
  end
  
  def post(url, params={})
    Net::HTTP.post_form(URI.parse("http://#{@server.host}:#{@server.port}" + url), params).body
  end
end

Spec::Runner.configure do |config|
  config.include Matchers
  config.include Helpers
end