/*****************************************************************************
* 
* Nagios-plugins extra_opts library
* 
* License: GPL
* Copyright (c) 2007 Nagios Plugins Development Team
* 
* Last Modified: $Date: 2008-03-15 18:42:01 -0400 (Sat, 15 Mar 2008) $
* 
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* 
* $Id: parse_ini.c 1950 2008-03-15 22:42:01Z dermoth $
* 
*****************************************************************************/

#include "common.h"
#include "utils_base.h"
#include "parse_ini.h"
#include "extra_opts.h"

/* FIXME: copied from utils.h; we should move a bunch of libs! */
int
is_option (char *str)
{
	if (!str)
		return 0;
	else if (strspn (str, "-") == 1 || strspn (str, "-") == 2)
		return 1;
	else
		return 0;
}

/* this is the externally visible function used by plugins */
char **np_extra_opts(int *argc, char **argv, const char *plugin_name){
	np_arg_list *extra_args=NULL, *ea1=NULL, *ea_tmp=NULL;
	char **argv_new=NULL;
	char *argptr=NULL;
	int i, j, optfound, argc_new, ea_num=*argc;

	if(*argc<2) {
		/* No arguments provided */
		return argv;
	}

	for(i=1; i<*argc; i++){
		argptr=NULL;
		optfound=0;

		/* Do we have an extra-opts parameter? */
		if(strncmp(argv[i], "--extra-opts=", 13)==0){
			/* It is a single argument with value */
			argptr=argv[i]+13;
			/* Delete the extra opts argument */
			for(j=i;j<*argc;j++) argv[j]=argv[j+1];
			i--;
			*argc-=1;
		}else if(strcmp(argv[i], "--extra-opts")==0){
			if(!is_option(argv[i+1])){
				/* It is a argument with separate value */
				argptr=argv[i+1];
				/* Delete the extra-opts argument/value */
				for(j=i;j<*argc-1;j++) argv[j]=argv[j+2];
				i-=2;
				*argc-=2;
				ea_num--;
			}else{
				/* It has no value */
				optfound=1;
				/* Delete the extra opts argument */
				for(j=i;j<*argc;j++) argv[j]=argv[j+1];
				i--;
				*argc-=1;
			}
		}

		/* If we found extra-opts, expand them and store them for later*/
		if(argptr||optfound){
			/* Process ini section, returning a linked list of arguments */
			ea1=np_get_defaults(argptr, plugin_name);
			if(ea1==NULL) {
				/* no extra args (empty section)? */
				ea_num--;
				continue;
			}

			/* append the list to extra_args */
			if(extra_args==NULL){
				extra_args=ea1;
				while(ea1=ea1->next) ea_num++;
			}else{
				ea_tmp=extra_args;
				while(ea_tmp->next) {
					ea_tmp=ea_tmp->next;
					ea_num++;
				}
				ea_tmp->next=ea1;
			}
			ea1=ea_tmp=NULL;
		}
		/* lather, rince, repeat */
	}

	if(ea_num==*argc && extra_args==NULL){
		/* No extra-opts */
		return argv;
	}

	/* done processing arguments. now create a new argv array... */
	argv_new=(char**)malloc((ea_num+1)*sizeof(char**));
	if(argv_new==NULL) die(STATE_UNKNOWN, _("malloc() failed!\n"));

	/* starting with program name */
	argv_new[0]=strdup(argv[0]);
	argc_new=1;
	/* then parsed ini opts (frying them up in the same run) */
	while(extra_args){
		argv_new[argc_new++]=extra_args->arg;
		ea1=extra_args;
		extra_args=extra_args->next;
		free(ea1);
	}
	/* finally the rest of the argv array */
	for (i=1; i<*argc; i++)	argv_new[argc_new++]=strdup(argv[i]);
	*argc=argc_new;
	/* and terminate. */
	argv_new[argc_new]=NULL;

	return argv_new;
}

