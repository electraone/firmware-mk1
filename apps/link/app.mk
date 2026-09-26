#
# The Electra Link program borrows the controller's components, model and
# windows rather than drawing lookalikes of them: a page on the second screen
# is painted by the very code that paints it on the first. Everything under
# apps/controller/src is compiled in except ControllerApp.cpp, which defines
# the controller's own createApplication(). Whatever the link never calls is
# dropped by the linker's section garbage collection.
#
LINK_BORROWED := $(filter-out $(wildcard apps/controller/src/*.cpp), \
		$(wildcard apps/controller/src/*/*.cpp))

APPCPP_FILES += $(LINK_BORROWED)

L_APP_INC += -Iapps/controller/src \
		$(foreach x,$(filter %/, $(wildcard apps/controller/src/*/)), -I$(x))
