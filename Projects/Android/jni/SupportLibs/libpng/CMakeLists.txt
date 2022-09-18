SET(PNG_DIR  ${SRC_ROOT}/libpng)
include_directories(${PNG_DIR})
SET(PNG_SRC_FILES 
	${PNG_DIR}/png.c 
	${PNG_DIR}/pngerror.c 
	${PNG_DIR}/pngget.c 
	${PNG_DIR}/pngmem.c 
	${PNG_DIR}/pngpread.c 
	${PNG_DIR}/pngread.c 
	${PNG_DIR}/pngrio.c 
	${PNG_DIR}/pngrtran.c 
	${PNG_DIR}/pngrutil.c 
	${PNG_DIR}/pngset.c 
	${PNG_DIR}/pngtrans.c 
	${PNG_DIR}/pngwio.c 
	${PNG_DIR}/pngwrite.c 
	${PNG_DIR}/pngwtran.c 
	${PNG_DIR}/pngwutil.c 
	${PNG_DIR}/arm/arm_init.c  
	${PNG_DIR}/arm/filter_neon_intrinsics.c
)


