#ifndef __GCAL_LIB__
#define __GCAL_LIB__


struct gcal_resource;

void gcal_destroy(struct gcal_resource *gcal_obj);

struct gcal_resource *gcal_initialize(void);

int gcal_get_authentication(char *user, char *password,
			    struct gcal_resource *ptr_gcal);


#endif
