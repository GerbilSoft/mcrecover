/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * git.h: Git version macros.                                              *
 *                                                                         *
 * Copyright (c) 2008-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// git_version.h is generated by git_version.sh
#include "git_version.h"

// MCRECOVER_GIT_VERSION: Macro for the git revision, if available.
#ifdef GIT_REPO
	#ifdef GIT_BRANCH
		#define MCRECOVER_GIT_TMP_BRANCH GIT_BRANCH
		#ifdef GIT_SHAID
			#define MCRECOVER_GIT_TMP_SHAID "/" GIT_SHAID
		#else /* !GIT_SHAID */
			#define MCRECOVER_GIT_TMP_SHAID
		#endif /* GIT_SHAID */
	#else /* !GIT_BRANCH */
		#define MCRECOVER_GIT_TMP_BRANCH
		#ifdef GIT_SHAID
			#define MCRECOVER_GIT_TMP_SHAID GIT_SHAID
		#else /* !GIT_SHAID */
			#define MCRECOVER_GIT_TMP_SHAID
		#endif /* GIT_SHAID */
	#endif /* GIT_BRANCH */
	
	#ifdef GIT_DIRTY
		#define MCRECOVER_GIT_TMP_DIRTY "+"
	#else /* !GIT_DIRTY */
		#define MCRECOVER_GIT_TMP_DIRTY
	#endif /* GIT_DIRTY */
	
	#define MCRECOVER_GIT_VERSION "git: " MCRECOVER_GIT_TMP_BRANCH MCRECOVER_GIT_TMP_SHAID MCRECOVER_GIT_TMP_DIRTY
	#ifdef GIT_DESCRIBE
		#define MCRECOVER_GIT_DESCRIBE GIT_DESCRIBE MCRECOVER_GIT_TMP_DIRTY
	#endif
#else /* !GIT_REPO */
	#ifdef MCRECOVER_GIT_VERSION
		#undef MCRECOVER_GIT_VERSION
	#endif /* MCRECOVER_GIT_VERSION */
	#ifdef MCRECOVER_GIT_DESCRIBE
		#undef MCRECOVER_GIT_DESCRIBE
	#endif /* MCRECOVER_GIT_DESCRIBE */
#endif /* GIT_REPO */
