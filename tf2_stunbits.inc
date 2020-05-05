#if defined _STUNBALL_BITS
 #endinput
#endif
#define _STUNBALL_BITS

/**
 * @brief	Called whenever a stunball touch an enemy
 *
 *	@param client	ball's owner
 * @param victim	target
 * @param bits		refer to tf2.inc for STUN_FLAG
 *
 * @return			Plugin_Stop to completly nullify bits
 *						any value other than Plugin_Continue won't ignore the bits param
 */
forward Action TF2_OnApplyStunBallEffect(int client, int victim, int &bits);

public SharedPlugin __TF2_STUNBALL =
{
	name = "Override StunBall",
	file = "tf2_stun.smx",
	#if defined REQUIRE_PLUGIN
	required = 1,
	#else
	required = 0,
	#endif
};
