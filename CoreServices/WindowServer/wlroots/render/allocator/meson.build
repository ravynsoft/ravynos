wlr_files += files(
	'allocator.c',
	'gbm.c',
	'shm.c',
	'drm_dumb.c',
)

has = cc.has_function('gbm_bo_get_fd_for_plane', dependencies: [gbm])
add_project_arguments('-DHAS_GBM_BO_GET_FD_FOR_PLANE=@0@'.format(has.to_int()), language: 'c')
