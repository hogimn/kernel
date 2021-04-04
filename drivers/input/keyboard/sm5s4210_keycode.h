/*
 *	
 * SM5S4210 Dev Board key-pad header file 
 *
 */
#ifndef	_SM5S4210_KEYCODE_H_

#define	_SM5S4210_KEYCODE_H_
	#if defined(CONFIG_KEYPAD_SM5S4210)
		 #define	MAX_KEYCODE_CNT		3
	
		int SM5S4210_Keycode[MAX_KEYCODE_CNT] = {
			KEY_VOLUMEUP,	KEY_VOLUMEDOWN,	KEY_POWER
		};
	
		#if	defined(DEBUG_MSG)
			const char SM5S4210_KeyMapStr[MAX_KEYCODE_CNT][20] = {
				"KEY_VOLUMEUP\n",	"KEY_VOLUMEDOWN\n",	"KEY_POWER\n"
			};
		#endif	// DEBUG_MSG
	#endif	

#endif		/* _SM5S4210_KEYPAD_H_*/
