module Thin
  module Backends
    class Turbo
      include Logging
      
      # Server serving the connections throught the backend
      attr_accessor :server
      
      # Address and port on which the server is listening for connections.
      attr_accessor :host, :port
      
      # Maximum time for incoming data to arrive
      attr_accessor :timeout
      
      # Maximum number of file or socket descriptors that the server may open.
      attr_accessor :maximum_connections
      
      # Maximum number of connections that can be persistent
      attr_accessor :maximum_persistent_connections
      
      # Allow using threads in the backend.
      attr_writer :threaded
      def threaded?; @threaded end
      
      # Number of persistent connections currently opened
      attr_accessor :persistent_connection_count
      
      def initialize(host, port, options={})
        @host = host
        @port = port.to_i
      end
      
      def start
        @app = @server.app
        listen_on_port(@host, @port)

        @running = true
        loop! while @running
        close
      end

      def stop
        @running = false
      end
      alias :stop! :stop
    
      def config
        self.maxfds = @maximum_connections
        @maximum_connections = maxfds
      end
    
      def running?
        @running
      end
    
      def empty?
        size.zero?
      end
    
      def size
        0 # TODO
      end
      
      def to_s
        "#{@host}:#{@port} (turbo)"
      end
      
      protected
        def log_last_exception
          log "!! Unexpected error while processing request: #{$!}"
          log $!.backtrace.join("\n")
        end
      
        def log_error(msg, file=nil, function=nil, line=nil)
          log "!! Unexpected error while processing request: #{msg}"
          log "   in " + [file, function, line].compact.join(", ")
        end
    end
  end
end
