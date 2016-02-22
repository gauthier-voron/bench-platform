#ifndef OVERRIDE_H
#define OVERRIDE_H


#include <dlfcn.h>
#include <stdio.h>


#define GENERATE_OVERRIDE_STUB_PART_0(ret, name, ...)			\
	static ret __load_##name(__VA_ARGS__);				\
									\
	static ret (*__##name)(__VA_ARGS__) =				\
		__load_##name;						\
									\
	static ret __load_##name(__VA_ARGS__)				\
	{								\
		int (*__local)(__VA_ARGS__);				\
									\
		__local = dlsym(RTLD_NEXT, #name);			\
									\
		if (__local_##name == NULL) {				\
			fprintf(stderr, "chronometer: warning: cannot "	\
				"find symbol '" #name "'\n");		\
			exit(EXIT_FAILURE);				\
		}							\
									\
		__sync_bool_compare_and_swap(&__##name,			\
					     __load_##name,		\
					     __local);			\
									\
		GENERATE_OVERRIDE_STUB_PART_1

#define GENERATE_OVERRIDE_STUB_PART_1(ret, name, ...)		        \
	        return __##name(__VA_ARGS__);			        \
	}						                \



#define OVERRIDE(ret, name, call, proto)				\
	static ret __load_##name proto;					\
									\
	static ret (*__##name) proto =					\
		__load_##name;						\
									\
	static ret __load_##name proto					\
	{								\
		int (*__local) proto;					\
									\
		__local = dlsym(RTLD_NEXT, #name);			\
									\
		if (__local == NULL) {					\
			fprintf(stderr,	"chronometer: warning: cannot " \
				"find symbol '" #name "'\n");		\
			exit(EXIT_FAILURE);				\
		}							\
									\
		__sync_bool_compare_and_swap(&__##name,			\
					     __load_##name,		\
					     __local);			\
									\
		return __##name call;					\
	}								\
									\
	ret name proto							\


#define ORIGINAL(name)		__##name


#define OVERRIDE_VERSION(version, alias, ret, name, call, proto)	\
	__OVERRIDE_VERSION("@", version, alias, ret, name, call, proto)

#define OVERRIDE_DEFAULT(version, alias, ret, name, call, proto)	\
	__OVERRIDE_VERSION("@@", version, alias, ret, name, call, proto)

#define __OVERRIDE_VERSION(def, version, alias, ret, name, call, proto) \
	static ret __load_##name##_##alias proto;			\
									\
	static ret (*__##name##_##alias) proto =			\
		__load_##name##_##alias;				\
									\
	static ret __load_##name##_##alias proto			\
	{								\
		int (*__local) proto;					\
									\
		__local = dlvsym(RTLD_NEXT, #name, #version);		\
									\
		if (__local == NULL) {					\
			fprintf(stderr,	"chronometer: warning: cannot " \
				"find symbol '" #name "' version '"	\
				#version "'\n");			\
			exit(EXIT_FAILURE);				\
		}							\
									\
		__sync_bool_compare_and_swap(&__##name##_##alias,	\
					     __load_##name##_##alias,	\
					     __local);			\
									\
		return __##name##_##alias call;				\
	}								\
									\
	ret name##_##alias proto;					\
									\
	asm(".symver " #name "_" #alias ", " #name def #version);	\
									\
	ret name##_##alias proto					\


#define ORIGINAL_VERSION(alias, name)		__##name##_##alias


#endif
