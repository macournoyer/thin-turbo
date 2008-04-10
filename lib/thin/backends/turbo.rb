module Thin
  module Backends
    class Turbo
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
        self.app = @server.app
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
      end
    
      def running?
        @running
      end
    
      def empty?
        false # TODO
      end
    
      def size
        0 # TODO
      end
      
      def to_s
        "#{@host}:#{@port} (turbo)"
      end
      
      protected
        def log_error(ex=$!)
          puts "!! Unexpected error while processing request: #{ex.message}"
          puts ex.backtrace.join("\n") if ex.backtrace
        end
    end
  end
end
