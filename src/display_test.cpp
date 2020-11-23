#include "display.hpp" 
#include <memory>

int main(int argc, char *argv[])
{
	auto disp = std::make_unique<Display>();
	disp->drawBlock(31,31);
	disp->close();
	return 0;
}
