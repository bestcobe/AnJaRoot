/*
 * Copyright 2013 Simon Brakhane
 *
 * This file is part of AnJaRoot.
 *
 * AnJaRoot is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * AnJaRoot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * AnJaRoot. If not, see http://www.gnu.org/licenses/.
 */

#ifndef _ANJAROOT_LIB_COMPAT_H_
#define _ANJAROOT_LIB_COMPAT_H_

#include <jni.h>

extern bool initializeLibrary(JNIEnv* env);
extern bool isSetCapCompatEnabled();

jintArray jni_getversion(JNIEnv* env, jclass cls);
void jni_setcompatmode(JNIEnv*, jclass cls, jint apilvl);

#endif