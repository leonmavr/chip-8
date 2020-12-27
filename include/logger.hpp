#ifndef LOGGER_HPP
#define LOGGER_HPP 

class Logger {
	private:
		Logger() {}
		int iterations = 0;

	public:
		static Logger& getInstance(){
			static Logger instance;
			return instance;
		}
		int log();
};

#endif /* LOGGER_HPP */
