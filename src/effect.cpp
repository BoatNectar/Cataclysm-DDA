#include "effect.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <unordered_set>

#include "bodypart.h"
#include "color.h"
#include "character.h"
#include "debug.h"
#include "effect_source.h"
#include "enums.h"
#include "event.h"
#include "generic_factory.h"
#include "json.h"
#include "messages.h"
#include "optional.h"
#include "output.h"
#include "rng.h"
#include "string_formatter.h"
#include "text_snippets.h"
#include "units.h"

static const efftype_id effect_bandaged( "bandaged" );
static const efftype_id effect_beartrap( "beartrap" );
static const efftype_id effect_crushed( "crushed" );
static const efftype_id effect_disinfected( "disinfected" );
static const efftype_id effect_downed( "downed" );
static const efftype_id effect_grabbed( "grabbed" );
static const efftype_id effect_heavysnare( "heavysnare" );
static const efftype_id effect_in_pit( "in_pit" );
static const efftype_id effect_lightsnare( "lightsnare" );
static const efftype_id effect_tied( "tied" );
static const efftype_id effect_webbed( "webbed" );
static const efftype_id effect_weed_high( "weed_high" );
static const efftype_id effect_worked_on( "worked_on" );

static const itype_id itype_holybook_bible( "holybook_bible" );
static const itype_id itype_money_one( "money_one" );

static const trait_id trait_LACTOSE( "LACTOSE" );
static const trait_id trait_VEGETARIAN( "VEGETARIAN" );

namespace
{
std::map<efftype_id, effect_type> effect_types;
} // namespace

void vitamin_rate_effect::load( const JsonObject &jo )
{
    mandatory( jo, false, "vitamin", vitamin );

    optional( jo, false, "rate", rate );
    optional( jo, false, "resist_rate", red_rate, rate );

    optional( jo, false, "absorb_mult", absorb_mult );
    optional( jo, false, "resist_absorb_mult", red_absorb_mult, absorb_mult );

    optional( jo, false, "tick", tick );
    optional( jo, false, "resist_tick", red_tick, tick );
}

void vitamin_rate_effect::deserialize( const JsonObject &jo )
{
    load( jo );
}

/** @relates string_id */
template<>
const effect_type &string_id<effect_type>::obj() const
{
    const auto iter = effect_types.find( *this );
    if( iter == effect_types.end() ) {
        debugmsg( "invalid effect type id %s", c_str() );
        static const effect_type dummy{};
        return dummy;
    }
    return iter->second;
}

/** @relates string_id */
template<>
bool string_id<effect_type>::is_valid() const
{
    return effect_types.count( *this ) > 0;
}

void weed_msg( Character &p )
{
    const time_duration howhigh = p.get_effect_dur( effect_weed_high );
    ///\EFFECT_INT changes messages when smoking weed
    int smarts = p.get_int();
    if( howhigh > 12_minutes && one_in( 7 ) ) {
        int msg = rng( 0, 5 );
        switch( msg ) {
            case 0:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Freakazoid_1" ).value_or(
                                         translation() ) );
                return;
            case 1:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Simpsons_1" ).value_or(
                                         translation() ) );
                p.mod_hunger( 2 );
                return;
            case 2:
                if( smarts > 8 ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Timothy_Leary" ).value_or(
                                             translation() ) );
                } else if( smarts < 3 ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_IASIF" ).value_or( translation() ) );
                } else {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Durr" ).value_or( translation() ) );
                }
                return;
            case 3:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Dazed_and_Confused_1" ).value_or(
                                         translation() ) );
                if( one_in( 2 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Dazed_and_Confused_2" ).value_or(
                                             translation() ) );
                    if( one_in( 2 ) ) {
                        p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Dazed_and_Confused_3" ).value_or(
                                                 translation() ) );
                    }
                }
                return;
            case 4:
                if( p.has_amount( itype_money_one, 1 ) ) { // Half Baked
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Half_Baked_1" ).value_or(
                                             translation() ) );
                    if( one_in( 2 ) ) {
                        p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Half_Baked_2" ).value_or(
                                                 translation() ) );
                        if( one_in( 3 ) ) {
                            p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Half_Baked_3" ).value_or(
                                                     translation() ) );
                        }
                    }
                } else if( p.has_amount( itype_holybook_bible, 1 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Bible" ).value_or( translation() ) );
                } else {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Big_Lebowski" ).value_or(
                                             translation() ) );
                }
                return;
            case 5:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Mitch_Hedberg" ).value_or(
                                         translation() ) );
            default:
                return;
        }
    } else if( howhigh > 10_minutes && one_in( 5 ) ) {
        int msg = rng( 0, 5 );
        switch( msg ) {
            case 0:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Bob_Marley" ).value_or(
                                         translation() ) );
                return;
            case 1:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Freakazoid_2" ).value_or(
                                         translation() ) );
                return;
            case 2:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Simpsons_2" ).value_or(
                                         translation() ) );
                if( smarts > 2 && one_in( 2 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Simpsons_3" ).value_or(
                                             translation() ) );
                }
                return;
            case 3:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Bill_Hicks" ).value_or(
                                         translation() ) );
                return;
            case 4:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Steve_Martin_1" ).value_or(
                                         translation() ) );
                if( one_in( 4 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Steve_Martin_2" ).value_or(
                                             translation() ) );
                }
                if( one_in( 4 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Steve_Martin_3" ).value_or(
                                             translation() ) );
                }
                if( one_in( 4 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Steve_Martin_4" ).value_or(
                                             translation() ) );
                }
                if( one_in( 4 ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Steve_Martin_5" ).value_or(
                                             translation() ) );
                }
                if( smarts > 2 ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Steve_Martin_6" ).value_or(
                                             translation() ) );
                }
                return;
            case 5:
            default:
                return;
        }
    } else if( howhigh > 5_minutes && one_in( 3 ) ) {
        int msg = rng( 0, 5 );
        switch( msg ) {
            case 0:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Cheech_and_Chong" ).value_or(
                                         translation() ) );
                return;
            case 1:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Real_Life_1" ).value_or(
                                         translation() ) );
                p.mod_hunger( 4 );
                if( p.has_trait( trait_VEGETARIAN ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Real_Life_2" ).value_or(
                                             translation() ) );
                } else if( p.has_trait( trait_LACTOSE ) ) {
                    p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Real_Life_3" ).value_or(
                                             translation() ) );
                }
                return;
            case 2:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Dazed_and_Confused_4" ).value_or(
                                         translation() ) );
                return;
            case 3:
                p.add_msg_if_player( "%s", SNIPPET.random_from_category( "weed_Half_Baked_4" ).value_or(
                                         translation() ) );
                return;
            case 4:
                // re-roll
                weed_msg( p );
            case 5:
            default:
                return;
        }
    }
}

static void extract_effect(
    const JsonObject &j,
    std::unordered_map<std::tuple<std::string, bool, std::string, std::string>, double,
    cata::tuple_hash> &data,
    const std::string &mod_type, const std::string &data_key,
    const std::string &type_key, const std::string &arg_key )
{
    double val = 0;
    double reduced_val = 0;
    if( j.has_member( mod_type ) ) {
        JsonArray jsarr = j.get_array( mod_type );
        val = jsarr.get_float( 0 );
        // If a second value exists use it, else reduced_val = val.
        if( jsarr.size() >= 2 ) {
            reduced_val = jsarr.get_float( 1 );
        } else {
            reduced_val = val;
        }
    }
    // Store values if they aren't zero.
    if( val != 0 ) {
        data[std::make_tuple( data_key, false, type_key, arg_key )] = val;
    }
    if( reduced_val != 0 ) {
        data[std::make_tuple( data_key, true, type_key, arg_key )] = reduced_val;
    }
}

bool effect_type::load_mod_data( const JsonObject &jo, const std::string &member )
{
    if( jo.has_object( member ) ) {
        JsonObject j = jo.get_object( member );

        // Stats first
        //                          json field                  type key    arg key
        extract_effect( j, mod_data, "str_mod",          member, "STR",      "min" );
        extract_effect( j, mod_data, "dex_mod",          member, "DEX",      "min" );
        extract_effect( j, mod_data, "per_mod",          member, "PER",      "min" );
        extract_effect( j, mod_data, "int_mod",          member, "INT",      "min" );
        extract_effect( j, mod_data, "speed_mod",        member, "SPEED",    "min" );

        // Then pain
        extract_effect( j, mod_data, "pain_amount",      member, "PAIN",     "amount" );
        extract_effect( j, mod_data, "pain_min",         member, "PAIN",     "min" );
        extract_effect( j, mod_data, "pain_max",         member, "PAIN",     "max" );
        extract_effect( j, mod_data, "pain_max_val",     member, "PAIN",     "max_val" );
        extract_effect( j, mod_data, "pain_chance",      member, "PAIN",     "chance_top" );
        extract_effect( j, mod_data, "pain_chance_bot",  member, "PAIN",     "chance_bot" );
        extract_effect( j, mod_data, "pain_tick",        member, "PAIN",     "tick" );

        // Then hurt
        extract_effect( j, mod_data, "hurt_amount",      member, "HURT",     "amount" );
        extract_effect( j, mod_data, "hurt_min",         member, "HURT",     "min" );
        extract_effect( j, mod_data, "hurt_max",         member, "HURT",     "max" );
        extract_effect( j, mod_data, "hurt_chance",      member, "HURT",     "chance_top" );
        extract_effect( j, mod_data, "hurt_chance_bot",  member, "HURT",     "chance_bot" );
        extract_effect( j, mod_data, "hurt_tick",        member, "HURT",     "tick" );

        // Then sleep
        extract_effect( j, mod_data, "sleep_amount",     member, "SLEEP",    "amount" );
        extract_effect( j, mod_data, "sleep_min",        member, "SLEEP",    "min" );
        extract_effect( j, mod_data, "sleep_max",        member, "SLEEP",    "max" );
        extract_effect( j, mod_data, "sleep_chance",     member, "SLEEP",    "chance_top" );
        extract_effect( j, mod_data, "sleep_chance_bot", member, "SLEEP",    "chance_bot" );
        extract_effect( j, mod_data, "sleep_tick",       member, "SLEEP",    "tick" );

        // Then pkill
        extract_effect( j, mod_data, "pkill_amount",     member, "PKILL",    "amount" );
        extract_effect( j, mod_data, "pkill_min",        member, "PKILL",    "min" );
        extract_effect( j, mod_data, "pkill_max",        member, "PKILL",    "max" );
        extract_effect( j, mod_data, "pkill_max_val",    member, "PKILL",    "max_val" );
        extract_effect( j, mod_data, "pkill_chance",     member, "PKILL",    "chance_top" );
        extract_effect( j, mod_data, "pkill_chance_bot", member, "PKILL",    "chance_bot" );
        extract_effect( j, mod_data, "pkill_tick",       member, "PKILL",    "tick" );

        // Then stim
        extract_effect( j, mod_data, "stim_amount",      member, "STIM",     "amount" );
        extract_effect( j, mod_data, "stim_min",         member, "STIM",     "min" );
        extract_effect( j, mod_data, "stim_max",         member, "STIM",     "max" );
        extract_effect( j, mod_data, "stim_min_val",     member, "STIM",     "min_val" );
        extract_effect( j, mod_data, "stim_max_val",     member, "STIM",     "max_val" );
        extract_effect( j, mod_data, "stim_chance",      member, "STIM",     "chance_top" );
        extract_effect( j, mod_data, "stim_chance_bot",  member, "STIM",     "chance_bot" );
        extract_effect( j, mod_data, "stim_tick",        member, "STIM",     "tick" );

        // Then health
        extract_effect( j, mod_data, "health_amount",    member, "HEALTH",   "amount" );
        extract_effect( j, mod_data, "health_min",       member, "HEALTH",   "min" );
        extract_effect( j, mod_data, "health_max",       member, "HEALTH",   "max" );
        extract_effect( j, mod_data, "health_min_val",   member, "HEALTH",   "min_val" );
        extract_effect( j, mod_data, "health_max_val",   member, "HEALTH",   "max_val" );
        extract_effect( j, mod_data, "health_chance",    member, "HEALTH",   "chance_top" );
        extract_effect( j, mod_data, "health_chance_bot", member, "HEALTH",   "chance_bot" );
        extract_effect( j, mod_data, "health_tick",      member, "HEALTH",   "tick" );

        // Then health mod
        extract_effect( j, mod_data, "h_mod_amount",     member, "H_MOD",    "amount" );
        extract_effect( j, mod_data, "h_mod_min",        member, "H_MOD",    "min" );
        extract_effect( j, mod_data, "h_mod_max",        member, "H_MOD",    "max" );
        extract_effect( j, mod_data, "h_mod_min_val",    member, "H_MOD",    "min_val" );
        extract_effect( j, mod_data, "h_mod_max_val",    member, "H_MOD",    "max_val" );
        extract_effect( j, mod_data, "h_mod_chance",     member, "H_MOD",    "chance_top" );
        extract_effect( j, mod_data, "h_mod_chance_bot", member, "H_MOD",    "chance_bot" );
        extract_effect( j, mod_data, "h_mod_tick",       member, "H_MOD",    "tick" );

        // Then radiation
        extract_effect( j, mod_data, "rad_amount",       member, "RAD",      "amount" );
        extract_effect( j, mod_data, "rad_min",          member, "RAD",      "min" );
        extract_effect( j, mod_data, "rad_max",          member, "RAD",      "max" );
        extract_effect( j, mod_data, "rad_max_val",      member, "RAD",      "max_val" );
        extract_effect( j, mod_data, "rad_chance",       member, "RAD",      "chance_top" );
        extract_effect( j, mod_data, "rad_chance_bot",   member, "RAD",      "chance_bot" );
        extract_effect( j, mod_data, "rad_tick",         member, "RAD",      "tick" );

        // Then hunger
        extract_effect( j, mod_data, "hunger_amount",    member, "HUNGER",   "amount" );
        extract_effect( j, mod_data, "hunger_min",       member, "HUNGER",   "min" );
        extract_effect( j, mod_data, "hunger_max",       member, "HUNGER",   "max" );
        extract_effect( j, mod_data, "hunger_min_val",   member, "HUNGER",   "min_val" );
        extract_effect( j, mod_data, "hunger_max_val",   member, "HUNGER",   "max_val" );
        extract_effect( j, mod_data, "hunger_chance",    member, "HUNGER",   "chance_top" );
        extract_effect( j, mod_data, "hunger_chance_bot", member, "HUNGER",   "chance_bot" );
        extract_effect( j, mod_data, "hunger_tick",      member, "HUNGER",   "tick" );

        // Then thirst
        extract_effect( j, mod_data, "thirst_amount",    member, "THIRST",   "amount" );
        extract_effect( j, mod_data, "thirst_min",       member, "THIRST",   "min" );
        extract_effect( j, mod_data, "thirst_max",       member, "THIRST",   "max" );
        extract_effect( j, mod_data, "thirst_min_val",   member, "THIRST",   "min_val" );
        extract_effect( j, mod_data, "thirst_max_val",   member, "THIRST",   "max_val" );
        extract_effect( j, mod_data, "thirst_chance",    member, "THIRST",   "chance_top" );
        extract_effect( j, mod_data, "thirst_chance_bot", member, "THIRST",   "chance_bot" );
        extract_effect( j, mod_data, "thirst_tick",      member, "THIRST",   "tick" );

        // Then thirst
        extract_effect( j, mod_data, "perspiration_amount",    member, "PERSPIRATION",   "amount" );
        extract_effect( j, mod_data, "perspiration_min",       member, "PERSPIRATION",   "min" );
        extract_effect( j, mod_data, "perspiration_max",       member, "PERSPIRATION",   "max" );
        extract_effect( j, mod_data, "perspiration_min_val",   member, "PERSPIRATION",   "min_val" );
        extract_effect( j, mod_data, "perspiration_max_val",   member, "PERSPIRATION",   "max_val" );
        extract_effect( j, mod_data, "perspiration_chance",    member, "PERSPIRATION",   "chance_top" );
        extract_effect( j, mod_data, "perspiration_chance_bot", member, "PERSPIRATION",   "chance_bot" );
        extract_effect( j, mod_data, "perspiration_tick",      member, "PERSPIRATION",   "tick" );

        // Then fatigue
        extract_effect( j, mod_data, "fatigue_amount",    member, "FATIGUE",  "amount" );
        extract_effect( j, mod_data, "fatigue_min",       member, "FATIGUE",  "min" );
        extract_effect( j, mod_data, "fatigue_max",       member, "FATIGUE",  "max" );
        extract_effect( j, mod_data, "fatigue_min_val",   member, "FATIGUE",  "min_val" );
        extract_effect( j, mod_data, "fatigue_max_val",   member, "FATIGUE",  "max_val" );
        extract_effect( j, mod_data, "fatigue_chance",    member, "FATIGUE",  "chance_top" );
        extract_effect( j, mod_data, "fatigue_chance_bot", member, "FATIGUE",  "chance_bot" );
        extract_effect( j, mod_data, "fatigue_tick",      member, "FATIGUE",  "tick" );

        // Then stamina
        extract_effect( j, mod_data, "stamina_amount",    member, "STAMINA",  "amount" );
        extract_effect( j, mod_data, "stamina_min",       member, "STAMINA",  "min" );
        extract_effect( j, mod_data, "stamina_max",       member, "STAMINA",  "max" );
        extract_effect( j, mod_data, "stamina_max_val",   member, "STAMINA",  "max_val" );
        extract_effect( j, mod_data, "stamina_chance",    member, "STAMINA",  "chance_top" );
        extract_effect( j, mod_data, "stamina_chance_bot", member, "STAMINA",  "chance_bot" );
        extract_effect( j, mod_data, "stamina_tick",      member, "STAMINA",  "tick" );

        // Then coughing
        extract_effect( j, mod_data, "cough_chance",     member, "COUGH",    "chance_top" );
        extract_effect( j, mod_data, "cough_chance_bot", member, "COUGH",    "chance_bot" );
        extract_effect( j, mod_data, "cough_tick",       member, "COUGH",    "tick" );

        // Then vomiting
        extract_effect( j, mod_data, "vomit_chance",     member, "VOMIT",    "chance_top" );
        extract_effect( j, mod_data, "vomit_chance_bot", member, "VOMIT",    "chance_bot" );
        extract_effect( j, mod_data, "vomit_tick",       member, "VOMIT",    "tick" );

        // Then healing effects
        extract_effect( j, mod_data, "healing_rate",    member, "HEAL_RATE",  "amount" );
        extract_effect( j, mod_data, "healing_head",    member, "HEAL_HEAD",  "amount" );
        extract_effect( j, mod_data, "healing_torso",   member, "HEAL_TORSO", "amount" );

        // creature stats mod
        extract_effect( j, mod_data, "dodge_mod",    member, "DODGE",  "min" );
        extract_effect( j, mod_data, "hit_mod",    member, "HIT",  "min" );
        extract_effect( j, mod_data, "bash_mod",    member, "BASH",  "min" );
        extract_effect( j, mod_data, "cut_mod",    member, "CUT",  "min" );
        extract_effect( j, mod_data, "size_mod",    member, "SIZE",  "min" );

        return true;
    } else {
        return false;
    }
}

void effect_type::check_consistency()
{
    for( const std::pair<efftype_id, effect_type> check : effect_types ) {
        check.second.verify();
    }
}

void effect_type::verify() const
{
    if( death_event.has_value() ) {
        const std::unordered_map<std::string, cata_variant_type> &fields = cata::event::get_fields(
                    *death_event );
        // Only allow events inheriting from event_spec_character at the moment.
        if( fields.size() != 1 || fields.count( "character" ) != 1 ) {
            debugmsg( "Invalid death event type %s in effect_type %s", "lol",
                      io::enum_to_string( *death_event ),
                      id.str() );
        }
    }
}

bool effect_type::has_flag( const flag_id &flag ) const
{
    return flags.count( flag );
}

effect_rating effect_type::get_rating() const
{
    return rating;
}

bool effect_type::use_name_ints() const
{
    return name.size() > 1;
}

bool effect_type::use_desc_ints( bool reduced ) const
{
    if( reduced ) {
        return static_cast<size_t>( max_intensity ) <= reduced_desc.size();
    } else {
        return static_cast<size_t>( max_intensity ) <= desc.size();
    }
}

game_message_type effect_type::gain_game_message_type() const
{
    switch( rating ) {
        case e_good:
            return m_good;
        case e_bad:
            return m_bad;
        case e_neutral:
            return m_neutral;
        case e_mixed:
            return m_mixed;
        default:
            // Should never happen
            return m_neutral;
    }
}
game_message_type effect_type::lose_game_message_type() const
{
    switch( rating ) {
        case e_good:
            return m_bad;
        case e_bad:
            return m_good;
        case e_neutral:
            return m_neutral;
        case e_mixed:
            return m_mixed;
        default:
            // Should never happen
            return m_neutral;
    }
}
std::string effect_type::get_apply_message() const
{
    return apply_message.translated();
}
std::string effect_type::get_apply_memorial_log( const memorial_gender gender ) const
{
    switch( gender ) {
        case memorial_gender::male:
            return pgettext( "memorial_male", apply_memorial_log.c_str() );
        case memorial_gender::female:
            return pgettext( "memorial_female", apply_memorial_log.c_str() );
    }
    return std::string();
}
std::string effect_type::get_remove_message() const
{
    return remove_message.translated();
}
std::string effect_type::get_remove_memorial_log( const memorial_gender gender ) const
{
    switch( gender ) {
        case memorial_gender::male:
            return pgettext( "memorial_male", remove_memorial_log.c_str() );
        case memorial_gender::female:
            return pgettext( "memorial_female", remove_memorial_log.c_str() );
    }
    return std::string();
}
std::string effect_type::get_blood_analysis_description() const
{
    return blood_analysis_description.translated();
}
bool effect_type::get_main_parts() const
{
    return main_parts_only;
}
bool effect_type::is_show_in_info() const
{
    return show_in_info;
}
bool effect_type::load_miss_msgs( const JsonObject &jo, const std::string &member )
{
    return jo.read( member, miss_msgs );
}
bool effect_type::load_decay_msgs( const JsonObject &jo, const std::string &member )
{
    if( jo.has_array( member ) ) {
        for( JsonArray inner : jo.get_array( member ) ) {
            translation msg;
            inner.read( 0, msg );
            std::string r = inner.get_string( 1 );
            game_message_type rate = m_neutral;
            if( r == "good" ) {
                rate = m_good;
            } else if( r == "neutral" ) {
                rate = m_neutral;
            } else if( r == "bad" ) {
                rate = m_bad;
            } else if( r == "mixed" ) {
                rate = m_mixed;
            } else {
                inner.throw_error(
                    string_format( "Unexpected message type \"%s\"; expected \"good\", "
                                   "\"neutral\", " "\"bad\", or \"mixed\"", r ),
                    1 );
            }
            decay_msgs.emplace_back( msg, rate );
        }
        return true;
    }
    return false;
}

effect effect::null_effect;

bool effect::is_null() const
{
    return !eff_type;
}

std::string effect::disp_name() const
{
    if( eff_type->name.empty() ) {
        debugmsg( "No names for effect type, ID: %s", eff_type->id.c_str() );
        return "";
    }

    // End result should look like "name (l. arm)" or "name [intensity] (l. arm)"
    std::string ret;
    if( eff_type->use_name_ints() ) {
        const translation &d_name = eff_type->name[ std::min<size_t>( intensity,
                                                      eff_type->name.size() ) - 1 ];
        if( d_name.empty() ) {
            return std::string();
        }
        ret += d_name.translated();
    } else {
        if( eff_type->name[0].empty() ) {
            return std::string();
        }
        ret += eff_type->name[0].translated();
        if( intensity > 1 && eff_type->show_intensity ) {
            if( eff_type->id == effect_bandaged || eff_type->id == effect_disinfected ) {
                ret += string_format( " [%s]", texitify_healing_power( intensity ) );
            } else {
                ret += string_format( " [%d]", intensity );
            }
        }
    }
    if( bp != bodypart_str_id::NULL_ID() ) {
        ret += string_format( " (%s)", body_part_name( bp.id() ) );
    }

    return ret;
}

// Used in disp_desc()
struct desc_freq {
    double chance;
    int val;
    std::string pos_string;
    std::string neg_string;

    desc_freq( double c, int v, const std::string &pos, const std::string &neg ) : chance( c ),
        val( v ), pos_string( pos ), neg_string( neg ) {}
};

std::string effect::disp_desc( bool reduced ) const
{
    std::string ret;

    std::string timestr;
    time_duration effect_dur_elapsed = ( calendar::turn - start_time );
    if( to_turns<int>( effect_dur_elapsed ) == 0 ) {
        timestr = _( "just now" );
    } else {
        timestr = string_format( _( "%s ago" ),
                                 debug_mode ? to_string( effect_dur_elapsed ) : to_string_clipped( effect_dur_elapsed ) );
    }
    ret += string_format( _( "Effect started: <color_white>%s</color>" ), timestr );
    if( debug_mode ) {
        ret += string_format( _( "Effect ends in: <color_white>%s</color>" ), to_string( duration ) );
    }
    //Newline if necessary
    if( !ret.empty() && ret.back() != '\n' ) {
        ret += "\n";
    }

    // First print stat changes, adding + if value is positive
    int tmp = get_avg_mod( "STR", reduced );
    if( tmp > 0 ) {
        ret += string_format( _( "Strength <color_white>+%d</color>; " ), tmp );
    } else if( tmp < 0 ) {
        ret += string_format( _( "Strength <color_white>%d</color>; " ), tmp );
    }
    tmp = get_avg_mod( "DEX", reduced );
    if( tmp > 0 ) {
        ret += string_format( _( "Dexterity <color_white>+%d</color>; " ), tmp );
    } else if( tmp < 0 ) {
        ret += string_format( _( "Dexterity <color_white>%d</color>; " ), tmp );
    }
    tmp = get_avg_mod( "PER", reduced );
    if( tmp > 0 ) {
        ret += string_format( _( "Perception <color_white>+%d</color>; " ), tmp );
    } else if( tmp < 0 ) {
        ret += string_format( _( "Perception <color_white>%d</color>; " ), tmp );
    }
    tmp = get_avg_mod( "INT", reduced );
    if( tmp > 0 ) {
        ret += string_format( _( "Intelligence <color_white>+%d</color>; " ), tmp );
    } else if( tmp < 0 ) {
        ret += string_format( _( "Intelligence <color_white>%d</color>; " ), tmp );
    }
    tmp = get_avg_mod( "SPEED", reduced );
    if( tmp > 0 ) {
        ret += string_format( _( "Speed <color_white>+%d</color>; " ), tmp );
    } else if( tmp < 0 ) {
        ret += string_format( _( "Speed <color_white>%d</color>; " ), tmp );
    }
    // Newline if necessary
    if( !ret.empty() && ret.back() != '\n' ) {
        ret += "\n";
    }

    // Then print pain/damage/coughing/vomiting, we don't display pkill, health, or radiation
    std::vector<std::string> constant;
    std::vector<std::string> frequent;
    std::vector<std::string> uncommon;
    std::vector<std::string> rare;
    std::vector<desc_freq> values;
    // Add various desc_freq structs to values. If more effects wish to be placed in the descriptions this is the
    // place to add them.
    int val = 0;
    val = get_avg_mod( "PAIN", reduced );
    values.emplace_back( get_percentage( "PAIN", val, reduced ), val, _( "pain" ),
                         _( "pain" ) );
    val = get_avg_mod( "HURT", reduced );
    values.emplace_back( get_percentage( "HURT", val, reduced ), val, _( "damage" ),
                         _( "damage" ) );
    val = get_avg_mod( "STAMINA", reduced );
    values.emplace_back( get_percentage( "STAMINA", val, reduced ), val,
                         _( "stamina recovery" ), _( "fatigue" ) );
    val = get_avg_mod( "THIRST", reduced );
    values.emplace_back( get_percentage( "THIRST", val, reduced ), val, _( "thirst" ),
                         _( "quench" ) );
    val = get_avg_mod( "HUNGER", reduced );
    values.emplace_back( get_percentage( "HUNGER", val, reduced ), val, _( "hunger" ),
                         _( "sate" ) );
    val = get_avg_mod( "FATIGUE", reduced );
    values.emplace_back( get_percentage( "FATIGUE", val, reduced ), val, _( "sleepiness" ),
                         _( "rest" ) );
    val = get_avg_mod( "COUGH", reduced );
    values.emplace_back( get_percentage( "COUGH", val, reduced ), val, _( "coughing" ),
                         _( "coughing" ) );
    val = get_avg_mod( "VOMIT", reduced );
    values.emplace_back( get_percentage( "VOMIT", val, reduced ), val, _( "vomiting" ),
                         _( "vomiting" ) );
    val = get_avg_mod( "SLEEP", reduced );
    values.emplace_back( get_percentage( "SLEEP", val, reduced ), val, _( "blackouts" ),
                         _( "blackouts" ) );

    for( auto &i : values ) {
        if( i.val > 0 ) {
            // +50% chance, every other step
            if( i.chance >= 50.0 ) {
                constant.push_back( i.pos_string );
                // +1% chance, every 100 steps
            } else if( i.chance >= 1.0 ) {
                frequent.push_back( i.pos_string );
                // +.4% chance, every 250 steps
            } else if( i.chance >= .4 ) {
                uncommon.push_back( i.pos_string );
                // <.4% chance
            } else if( i.chance > 0 ) {
                rare.push_back( i.pos_string );
            }
        } else if( i.val < 0 ) {
            // +50% chance, every other step
            if( i.chance >= 50.0 ) {
                constant.push_back( i.neg_string );
                // +1% chance, every 100 steps
            } else if( i.chance >= 1.0 ) {
                frequent.push_back( i.neg_string );
                // +.4% chance, every 250 steps
            } else if( i.chance >= .4 ) {
                uncommon.push_back( i.neg_string );
                // <.4% chance
            } else if( i.chance > 0 ) {
                rare.push_back( i.neg_string );
            }
        }
    }
    if( !constant.empty() ) {
        ret += _( "Const: " ) + enumerate_as_string( constant ) + " ";
    }
    if( !frequent.empty() ) {
        ret += _( "Freq: " ) + enumerate_as_string( frequent ) + " ";
    }
    if( !uncommon.empty() ) {
        ret += _( "Unfreq: " ) + enumerate_as_string( uncommon ) + " ";
    }
    if( !rare.empty() ) {
        ret += _( "Rare: " ) + enumerate_as_string( rare ); // No space needed at the end
    }

    // Newline if necessary
    if( !ret.empty() && ret.back() != '\n' ) {
        ret += "\n";
    }

    std::string tmp_str;
    if( eff_type->use_desc_ints( reduced ) ) {
        if( reduced ) {
            tmp_str = eff_type->reduced_desc[intensity - 1].translated();
        } else {
            tmp_str = eff_type->desc[intensity - 1].translated();
        }
    } else {
        if( reduced ) {
            tmp_str = eff_type->reduced_desc[0].translated();
        } else {
            tmp_str = eff_type->desc[0].translated();
        }
    }
    // Then print the effect description
    if( use_part_descs() ) {
        ret += string_format( tmp_str, body_part_name( bp.id() ) );
    } else {
        if( !tmp_str.empty() ) {
            ret += tmp_str;
        }
    }

    if( debug_mode ) {
        ret += string_format(
                   _( "\nDEBUG: ID: <color_white>%s</color> Intensity: <color_white>%d</color>" ),
                   eff_type->id.c_str(), intensity );
    }

    return ret;
}

std::string effect::disp_short_desc( bool reduced ) const
{
    if( eff_type->use_desc_ints( reduced ) ) {
        if( reduced ) {
            return eff_type->reduced_desc[intensity - 1].translated();
        } else {
            return eff_type->desc[intensity - 1].translated();
        }
    } else {
        if( reduced ) {
            return eff_type->reduced_desc[0].translated();
        } else {
            return eff_type->desc[0].translated();
        }
    }
}

static bool effect_is_blocked( const efftype_id &e, const effects_map &eff_map )
{
    for( auto &eff_grp : eff_map ) {
        for( auto &eff : eff_grp.second ) {
            for( const efftype_id &block : eff.second.get_blocks_effects() ) {
                if( block == e ) {
                    return true;
                }
            }
        }
    }
    return false;
}

void effect::decay( std::vector<efftype_id> &rem_ids, std::vector<bodypart_id> &rem_bps,
                    const time_point &time, const bool player, const effects_map &eff_map )
{
    // Decay intensity if supposed to do so, removing effects at zero intensity
    if( intensity > 0 && eff_type->int_decay_tick != 0 &&
        to_turn<int>( time ) % eff_type->int_decay_tick == 0 &&
        get_max_duration() > get_duration() ) {
        if( eff_type->int_decay_step <= 0 || !effect_is_blocked( eff_type->id, eff_map ) ) {
            set_intensity( intensity + eff_type->int_decay_step, player );
        }
        if( intensity <= 0 ) {
            rem_ids.push_back( get_id() );
            rem_bps.push_back( bp.id() );
        }
    }

    // Add to removal list if duration is <= 0
    // Decay duration if not permanent
    if( duration <= 0_turns ) {
        rem_ids.push_back( get_id() );
        rem_bps.push_back( bp.id() );
    } else if( !is_permanent() ) {
        mod_duration( -1_turns, player );
    }
}

bool effect::use_part_descs() const
{
    return eff_type->part_descs;
}

time_duration effect::get_duration() const
{
    return duration;
}
time_duration effect::get_max_duration() const
{
    return eff_type->max_duration;
}
void effect::set_duration( const time_duration &dur, bool alert )
{
    duration = dur;
    // Cap to max_duration
    if( duration > eff_type->max_duration ) {
        duration = eff_type->max_duration;
    }

    // Force intensity if it is duration based
    if( eff_type->int_dur_factor != 0_turns ) {
        // + 1 here so that the lowest is intensity 1, not 0
        set_intensity( duration / eff_type->int_dur_factor + 1, alert );
    }

    add_msg_debug( debugmode::DF_EFFECT, "ID: %s, Duration %s", get_id().c_str(),
                   to_string_writable( duration ) );
}
void effect::mod_duration( const time_duration &dur, bool alert )
{
    set_duration( duration + dur, alert );
}
void effect::mult_duration( double dur, bool alert )
{
    set_duration( duration * dur, alert );
}

static int cap_to_size( const int max, int attempt )
{
    // Intensities start at 1, indexes at 0
    attempt -= 1;
    if( attempt >= max ) {
        return max - 1;
    } else {
        return attempt;
    }
}

static vitamin_applied_effect applied_from_rate( const bool reduced, const int intensity,
        const vitamin_rate_effect &vreff )
{
    vitamin_applied_effect added;
    added.vitamin = vreff.vitamin;

    if( reduced ) {
        if( !vreff.red_rate.empty() ) {
            const int idx = cap_to_size( vreff.red_rate.size(), intensity );
            added.rate = vreff.red_rate[idx];
        }
        if( !vreff.red_absorb_mult.empty() ) {
            const int idx = cap_to_size( vreff.red_absorb_mult.size(), intensity );
            added.absorb_mult = vreff.red_absorb_mult[idx];
        }
        if( !vreff.red_tick.empty() ) {
            const int idx = cap_to_size( vreff.red_tick.size(), intensity );
            added.tick = vreff.red_tick[idx];
        }
    } else {
        if( !vreff.rate.empty() ) {
            const int idx = cap_to_size( vreff.rate.size(), intensity );
            added.rate = vreff.rate[idx];
        }
        if( !vreff.absorb_mult.empty() ) {
            const int idx = cap_to_size( vreff.absorb_mult.size(), intensity );
            added.absorb_mult = vreff.absorb_mult[idx];
        }
        if( !vreff.tick.empty() ) {
            const int idx = cap_to_size( vreff.tick.size(), intensity );
            added.tick = vreff.tick[idx];
        }
    }

    return added;
}

std::vector<vitamin_applied_effect> effect::vit_effects( const bool reduced ) const
{
    std::vector<vitamin_applied_effect> ret;
    for( const vitamin_rate_effect &vreff : eff_type->vitamin_data ) {
        ret.push_back( applied_from_rate( reduced, intensity, vreff ) );
    }

    return ret;
}

time_point effect::get_start_time() const
{
    return start_time;
}

bodypart_id effect::get_bp() const
{
    return bp.id();
}
void effect::set_bp( const bodypart_str_id &part )
{
    bp = part;
}

bool effect::is_permanent() const
{
    return permanent;
}
void effect::pause_effect()
{
    permanent = true;
}
void effect::unpause_effect()
{
    permanent = false;
}

int effect::get_intensity() const
{
    return intensity;
}
int effect::get_max_intensity() const
{
    return eff_type->max_intensity;
}
int effect::get_max_effective_intensity() const
{
    return eff_type->max_effective_intensity;
}
int effect::get_effective_intensity() const
{
    if( eff_type->max_effective_intensity > 0 ) {
        return std::min( eff_type->max_effective_intensity, intensity );
    } else {
        return intensity;
    }
}

int effect::set_intensity( int val, bool alert )
{
    if( intensity < 1 ) {
        // Fix bad intensity
        add_msg_debug( debugmode::DF_EFFECT, "Bad intensity, ID: %s", get_id().c_str() );
        intensity = 1;
    }

    val = std::max( std::min( val, eff_type->max_intensity ), 0 );
    if( val == intensity ) {
        // Nothing to change
        return intensity;
    }

    // Filter out intensity falling to zero (the effect will be removed later)
    if( alert && val < intensity &&  val != 0 &&
        val - 1 < static_cast<int>( eff_type->decay_msgs.size() ) ) {
        add_msg( eff_type->decay_msgs[ val - 1 ].second,
                 eff_type->decay_msgs[ val - 1 ].first.translated() );
    }

    if( val == 0 && !eff_type->int_decay_remove ) {
        val = 1;
    }

    intensity = val;

    return intensity;
}

int effect::mod_intensity( int mod, bool alert )
{
    return set_intensity( intensity + mod, alert );
}

const std::vector<trait_id> &effect::get_resist_traits() const
{
    return eff_type->resist_traits;
}
const std::vector<efftype_id> &effect::get_resist_effects() const
{
    return eff_type->resist_effects;
}
const std::vector<efftype_id> &effect::get_removes_effects() const
{
    return eff_type->removes_effects;
}
std::vector<efftype_id> effect::get_blocks_effects() const
{
    std::vector<efftype_id> ret = eff_type->removes_effects;
    ret.insert( ret.end(), eff_type->blocks_effects.begin(), eff_type->blocks_effects.end() );
    return ret;
}

int effect::get_mod( const std::string &arg, bool reduced ) const
{
    const auto &mod_data = eff_type->mod_data;
    double min = 0;
    double max = 0;
    // Get the minimum total
    auto found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "min" ) );
    if( found != mod_data.end() ) {
        min += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "min" ) );
    if( found != mod_data.end() ) {
        min += found->second * ( get_effective_intensity() - 1 );
    }
    // Get the maximum total
    found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "max" ) );
    if( found != mod_data.end() ) {
        max += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "max" ) );
    if( found != mod_data.end() ) {
        max += found->second * ( get_effective_intensity() - 1 );
    }
    if( static_cast<int>( max ) != 0 ) {
        // Return a random value between [min, max]
        return static_cast<int>( rng( min, max ) );
    } else {
        // Else return the minimum value
        return min;
    }
}

int effect::get_avg_mod( const std::string &arg, bool reduced ) const
{
    const auto &mod_data = eff_type->mod_data;
    double min = 0;
    double max = 0;
    // Get the minimum total
    auto found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "min" ) );
    if( found != mod_data.end() ) {
        min += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "min" ) );
    if( found != mod_data.end() ) {
        min += found->second * ( get_effective_intensity() - 1 );
    }
    // Get the maximum total
    found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "max" ) );
    if( found != mod_data.end() ) {
        max += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "max" ) );
    if( found != mod_data.end() ) {
        max += found->second * ( get_effective_intensity() - 1 );
    }
    if( static_cast<int>( max ) != 0 ) {
        // Return an average of min and max
        return static_cast<int>( ( min + max ) / 2 );
    } else {
        // Else return the minimum value
        return min;
    }
}

int effect::get_amount( const std::string &arg, bool reduced ) const
{
    const auto &mod_data = eff_type->mod_data;
    double ret = 0;
    auto found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "amount" ) );
    if( found != mod_data.end() ) {
        ret += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "amount" ) );
    if( found != mod_data.end() ) {
        ret += found->second * ( get_effective_intensity() - 1 );
    }
    return static_cast<int>( ret );
}

int effect::get_min_val( const std::string &arg, bool reduced ) const
{
    const auto &mod_data = eff_type->mod_data;
    double ret = 0;
    auto found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "min_val" ) );
    if( found != mod_data.end() ) {
        ret += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "min_val" ) );
    if( found != mod_data.end() ) {
        ret += found->second * ( intensity - 1 );
    }
    return static_cast<int>( ret );
}

int effect::get_max_val( const std::string &arg, bool reduced ) const
{
    const auto &mod_data = eff_type->mod_data;
    double ret = 0;
    auto found = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "max_val" ) );
    if( found != mod_data.end() ) {
        ret += found->second;
    }
    found = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "max_val" ) );
    if( found != mod_data.end() ) {
        ret += found->second * ( intensity - 1 );
    }
    return static_cast<int>( ret );
}

bool effect::get_sizing( const std::string &arg ) const
{
    if( arg == "PAIN" ) {
        return eff_type->pain_sizing;
    } else if( arg == "HURT" ) {
        return eff_type->hurt_sizing;
    }
    return false;
}

double effect::get_percentage( const std::string &arg, int val, bool reduced ) const
{
    const auto &mod_data = eff_type->mod_data;
    auto found_top_base = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "chance_top" ) );
    auto found_top_scale = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg,
                                          "chance_top" ) );
    // Convert to int or 0
    int top_base = 0;
    int top_scale = 0;
    if( found_top_base != mod_data.end() ) {
        top_base = found_top_base->second;
    }
    if( found_top_scale != mod_data.end() ) {
        top_scale = found_top_scale->second * ( intensity - 1 );
    }
    // Check chances if value is 0 (so we can check valueless effects like vomiting)
    // Else a nonzero value overrides a 0 chance for default purposes
    if( val == 0 ) {
        // If both top values <= 0 then it should never trigger
        if( top_base <= 0 && top_scale <= 0 ) {
            return 0;
        }
        // It will also never trigger if top_base + top_scale <= 0
        if( top_base + top_scale <= 0 ) {
            return 0;
        }
    }

    // We only need to calculate these if we haven't already returned
    int bot_base = 0;
    int bot_scale = 0;
    int tick = 0;
    auto found_bot_base = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "chance_bot" ) );
    auto found_bot_scale = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg,
                                          "chance_bot" ) );
    auto found_tick_base = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "tick" ) );
    auto found_tick_scale = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "tick" ) );
    if( found_bot_base != mod_data.end() ) {
        bot_base = found_bot_base->second;
    }
    if( found_bot_scale != mod_data.end() ) {
        bot_scale = found_bot_scale->second * ( intensity - 1 );
    }
    if( found_tick_base != mod_data.end() ) {
        tick += found_tick_base->second;
    }
    if( found_tick_scale != mod_data.end() ) {
        tick += found_tick_scale->second * ( intensity - 1 );
    }
    // Tick is the exception where tick = 0 means tick = 1
    if( tick == 0 ) {
        tick = 1;
    }

    double ret = 0;
    // If both bot values are zero the formula is one_in(top), else the formula is x_in_y(top, bot)
    if( bot_base != 0 || bot_scale != 0 ) {
        if( bot_base + bot_scale == 0 ) {
            // Special crash avoidance case, in most effect fields 0 = "nothing happens"
            // so assume false here for consistency
            ret = 0;
        } else {
            // Cast to double here to allow for partial percentages
            ret = 100 * static_cast<double>( top_base + top_scale ) / static_cast<double>
                  ( bot_base + bot_scale );
        }
    } else {
        // Cast to double here to allow for partial percentages
        ret = 100 / static_cast<double>( top_base + top_scale );
    }
    // Divide by ticks between rolls
    if( tick > 1 ) {
        ret = ret / tick;
    }
    return ret;
}

bool effect::activated( const time_point &when, const std::string &arg, int val, bool reduced,
                        double mod ) const
{
    const auto &mod_data = eff_type->mod_data;
    auto found_top_base = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "chance_top" ) );
    auto found_top_scale = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg,
                                          "chance_top" ) );
    // Convert to int or 0
    int top_base = 0;
    int top_scale = 0;
    if( found_top_base != mod_data.end() ) {
        top_base = found_top_base->second;
    }
    if( found_top_scale != mod_data.end() ) {
        top_scale = found_top_scale->second * ( intensity - 1 );
    }
    // Check chances if value is 0 (so we can check valueless effects like vomiting)
    // Else a nonzero value overrides a 0 chance for default purposes
    if( val == 0 ) {
        // If both top values <= 0 then it should never trigger
        if( top_base <= 0 && top_scale <= 0 ) {
            return false;
        }
        // It will also never trigger if top_base + top_scale <= 0
        if( top_base + top_scale <= 0 ) {
            return false;
        }
    }

    // We only need to calculate these if we haven't already returned
    int bot_base = 0;
    int bot_scale = 0;
    int tick = 0;
    auto found_bot_base = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "chance_bot" ) );
    auto found_bot_scale = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg,
                                          "chance_bot" ) );
    auto found_tick_base = mod_data.find( std::make_tuple( "base_mods", reduced, arg, "tick" ) );
    auto found_tick_scale = mod_data.find( std::make_tuple( "scaling_mods", reduced, arg, "tick" ) );
    if( found_bot_base != mod_data.end() ) {
        bot_base = found_bot_base->second;
    }
    if( found_bot_scale != mod_data.end() ) {
        bot_scale = found_bot_scale->second * ( intensity - 1 );
    }
    if( found_tick_base != mod_data.end() ) {
        tick += found_tick_base->second;
    }
    if( found_tick_scale != mod_data.end() ) {
        tick += found_tick_scale->second * ( intensity - 1 );
    }
    // Tick is the exception where tick = 0 means tick = 1
    if( tick == 0 ) {
        tick = 1;
    }

    // Check if tick allows for triggering. If both bot values are zero the formula is
    // x_in_y(1, top) i.e. one_in(top), else the formula is x_in_y(top, bot),
    // mod multiplies the overall percentage chances

    // has to be an && here to avoid undefined behavior of turn % 0
    if( tick > 0 &&
        ( when - calendar::turn_zero ) % time_duration::from_turns( tick ) == 0_turns ) {
        if( bot_base != 0 && bot_scale != 0 ) {
            if( bot_base + bot_scale == 0 ) {
                // Special crash avoidance case, in most effect fields 0 = "nothing happens"
                // so assume false here for consistency
                return false;
            } else {
                return x_in_y( ( top_base + top_scale ) * mod, bot_base + bot_scale );
            }
        } else {
            return x_in_y( mod, top_base + top_scale );
        }
    }
    return false;
}

double effect::get_addict_mod( const std::string &arg, int addict_level ) const
{
    // TODO: convert this to JSON id's and values once we have JSON'ed addictions
    if( arg == "PKILL" ) {
        if( eff_type->pkill_addict_reduces ) {
            return 1.0 / std::max( static_cast<double>( addict_level ) * 2.0, 1.0 );
        } else {
            return 1.0;
        }
    } else {
        return 1.0;
    }
}

bool effect::get_harmful_cough() const
{
    return eff_type->harmful_cough;
}
int effect::get_dur_add_perc() const
{
    return eff_type->dur_add_perc;
}
time_duration effect::get_int_dur_factor() const
{
    return eff_type->int_dur_factor;
}
int effect::get_int_add_val() const
{
    return eff_type->int_add_val;
}

int effect::get_int_decay_step() const
{
    return eff_type->int_decay_step;
}

int effect::get_int_decay_tick() const
{
    return eff_type->int_decay_tick;
}

bool effect::get_int_decay_remove() const
{
    return eff_type->int_decay_remove;
}

const std::vector<std::pair<translation, int>> &effect::get_miss_msgs() const
{
    return eff_type->miss_msgs;
}
std::string effect::get_speed_name() const
{
    // USes the speed_mod_name if one exists, else defaults to the first entry in "name".
    // But make sure the name for this intensity actually exists!
    if( !eff_type->speed_mod_name.empty() ) {
        return eff_type->speed_mod_name.translated();
    } else if( eff_type->use_name_ints() ) {
        return eff_type->name[ std::min<size_t>( intensity, eff_type->name.size() ) - 1 ].translated();
    } else if( !eff_type->name.empty() ) {
        return eff_type->name[0].translated();
    } else {
        return "";
    }
}

bool effect::impairs_movement() const
{
    return eff_type->impairs_movement;
}

const effect_type *effect::get_effect_type() const
{
    return eff_type;
}

// This contains all the effects checked in move_effects
// It's here and not in json because it is hardcoded anyway
static const std::unordered_set<efftype_id> hardcoded_movement_impairing = {{
        effect_beartrap,
        effect_crushed,
        effect_downed,
        effect_grabbed,
        effect_heavysnare,
        effect_in_pit,
        effect_lightsnare,
        effect_tied,
        effect_webbed,
        effect_worked_on,
    }
};

void load_effect_type( const JsonObject &jo )
{
    effect_type new_etype;
    new_etype.id = efftype_id( jo.get_string( "id" ) );

    if( jo.has_member( "name" ) ) {
        for( const JsonValue entry : jo.get_array( "name" ) ) {
            translation name;
            if( !entry.read( name ) ) {
                entry.throw_error( "Error reading effect names" );
            }
            new_etype.name.emplace_back( name );
        }
    } else {
        new_etype.name.emplace_back();
    }
    jo.read( "speed_name", new_etype.speed_mod_name );

    if( jo.has_member( "desc" ) ) {
        jo.read( "desc", new_etype.desc );
    } else {
        new_etype.desc.emplace_back();
    }
    if( jo.has_member( "reduced_desc" ) ) {
        jo.read( "reduced_desc", new_etype.reduced_desc );
    } else {
        new_etype.reduced_desc = new_etype.desc;
    }

    new_etype.part_descs = jo.get_bool( "part_descs", false );

    if( jo.has_member( "rating" ) ) {
        std::string r = jo.get_string( "rating" );
        if( r == "good" ) {
            new_etype.rating = e_good;
        } else if( r == "neutral" ) {
            new_etype.rating = e_neutral;
        } else if( r == "bad" ) {
            new_etype.rating = e_bad;
        } else if( r == "mixed" ) {
            new_etype.rating = e_mixed;
        } else {
            jo.throw_error(
                string_format( "Unexpected rating \"%s\"; expected \"good\", \"neutral\", "
                               "\"bad\", or \"mixed\"", r ),
                "rating" );
        }
    } else {
        new_etype.rating = e_neutral;
    }
    jo.read( "apply_message", new_etype.apply_message );
    jo.read( "remove_message", new_etype.remove_message );
    optional( jo, false, "apply_memorial_log", new_etype.apply_memorial_log,
              text_style_check_reader() );
    optional( jo, false, "remove_memorial_log", new_etype.remove_memorial_log,
              text_style_check_reader() );

    jo.read( "blood_analysis_description", new_etype.blood_analysis_description );

    for( auto &&f : jo.get_string_array( "resist_traits" ) ) { // *NOPAD*
        new_etype.resist_traits.emplace_back( f );
    }
    for( auto &&f : jo.get_string_array( "resist_effects" ) ) { // *NOPAD*
        new_etype.resist_effects.emplace_back( f );
    }
    for( auto &&f : jo.get_string_array( "removes_effects" ) ) { // *NOPAD*
        new_etype.removes_effects.emplace_back( f );
    }
    for( auto &&f : jo.get_string_array( "blocks_effects" ) ) { // *NOPAD*
        new_etype.blocks_effects.emplace_back( f );
    }
    optional( jo, false, "max_duration", new_etype.max_duration, 365_days );

    if( jo.has_string( "int_dur_factor" ) ) {
        new_etype.int_dur_factor = read_from_json_string<time_duration>( jo.get_member( "int_dur_factor" ),
                                   time_duration::units );
    } else {
        new_etype.int_dur_factor = time_duration::from_turns( jo.get_int( "int_dur_factor", 0 ) );
    }

    optional( jo, false, "vitamins", new_etype.vitamin_data );
    optional( jo, false, "chance_kill", new_etype.kill_chance );
    optional( jo, false, "chance_kill_resist", new_etype.red_kill_chance );
    optional( jo, false, "death_msg", new_etype.death_msg, to_translation( "You died." ) );
    optional( jo, false, "death_event", new_etype.death_event,
              enum_flags_reader<event_type>( "event_type" ), cata::nullopt );

    new_etype.max_intensity = jo.get_int( "max_intensity", 1 );
    new_etype.dur_add_perc = jo.get_int( "dur_add_perc", 100 );
    new_etype.int_add_val = jo.get_int( "int_add_val", 0 );
    new_etype.int_decay_step = jo.get_int( "int_decay_step", -1 );
    new_etype.int_decay_tick = jo.get_int( "int_decay_tick", 0 );
    optional( jo, false, "int_decay_remove", new_etype.int_decay_remove, false );

    new_etype.load_miss_msgs( jo, "miss_messages" );
    new_etype.load_decay_msgs( jo, "decay_messages" );

    new_etype.main_parts_only = jo.get_bool( "main_parts_only", false );
    new_etype.show_in_info = jo.get_bool( "show_in_info", false );
    new_etype.show_intensity = jo.get_bool( "show_intensity", true );
    new_etype.pkill_addict_reduces = jo.get_bool( "pkill_addict_reduces", false );

    new_etype.pain_sizing = jo.get_bool( "pain_sizing", false );
    new_etype.hurt_sizing = jo.get_bool( "hurt_sizing", false );
    new_etype.harmful_cough = jo.get_bool( "harmful_cough", false );

    new_etype.max_effective_intensity = jo.get_int( "max_effective_intensity", 0 );

    new_etype.load_mod_data( jo, "base_mods" );
    new_etype.load_mod_data( jo, "scaling_mods" );

    new_etype.impairs_movement = hardcoded_movement_impairing.count( new_etype.id ) > 0;

    new_etype.flags = jo.get_tags<flag_id>( "flags" );

    effect_types[new_etype.id] = new_etype;
}

bool effect::has_flag( const flag_id &flag ) const
{
    return eff_type->has_flag( flag );
}

bool effect::kill_roll( bool reduced ) const
{
    const std::vector<std::pair<int, int>> &chances = reduced ? eff_type->red_kill_chance :
                                        eff_type->kill_chance;
    if( chances.empty() ) {
        return false;
    }

    const int idx = cap_to_size( chances.size(), intensity );
    const std::pair<int, int> &chance = chances[idx];
    return x_in_y( chance.first, chance.second );
}

std::string effect::get_death_message() const
{
    return eff_type->death_msg.translated();
}

event_type effect::death_event() const
{
    if( eff_type->death_event.has_value() ) {
        return *eff_type->death_event;
    }

    debugmsg( "Asked for death event from effect of type %s, but it lacks one!", eff_type->id.str() );
    return event_type::num_event_types;
}

void reset_effect_types()
{
    effect_types.clear();
}

const std::map<efftype_id, effect_type> &get_effect_types()
{
    return effect_types;
}

void effect_type::register_ma_buff_effect( const effect_type &eff )
{
    if( eff.id.is_valid() ) {
        debugmsg( "effect id %s of a martial art buff is already used as id for an effect",
                  eff.id.c_str() );
        return;
    }
    effect_types.insert( std::make_pair( eff.id, eff ) );
}

const effect_source &effect::get_source() const
{
    return this->source;
}

void effect::serialize( JsonOut &json ) const
{
    json.start_object();
    json.member( "eff_type", eff_type != nullptr ? eff_type->id.str() : "" );
    json.member( "duration", duration );
    json.member( "bp", bp.c_str() );
    json.member( "permanent", permanent );
    json.member( "intensity", intensity );
    json.member( "start_turn", start_time );
    json.member( "source", source );
    json.end_object();
}

void effect::deserialize( const JsonObject &jo )
{
    efftype_id id;
    jo.read( "eff_type", id );
    eff_type = &id.obj();
    jo.read( "duration", duration );

    // TEMPORARY until 0.F
    if( jo.has_int( "bp" ) ) {
        bp = convert_bp( static_cast<body_part>( jo.get_int( "bp" ) ) );
    } else {
        jo.read( "bp", bp );
    }

    jo.read( "permanent", permanent );
    jo.read( "intensity", intensity );
    start_time = calendar::turn_zero;
    jo.read( "start_turn", start_time );
    jo.read( "source", source );
}

std::string texitify_base_healing_power( const int power )
{
    if( power == 1 ) {
        return colorize( _( "very poor" ), c_red );
    } else if( power == 2 ) {
        return colorize( _( "poor" ), c_light_red );
    } else if( power == 3 ) {
        return colorize( _( "average" ), c_yellow );
    } else if( power == 4 ) {
        return colorize( _( "good" ), c_light_green );
    } else if( power >= 5 ) {
        return colorize( _( "great" ), c_green );
    }
    if( power < 1 ) {
        debugmsg( "Tried to convert zero or negative value." );
    }
    return "";
}

std::string texitify_healing_power( const int power )
{
    if( power == 0 ) {
        return colorize( _( "none" ), c_dark_gray );
    } else if( power >= 1 && power <= 2 ) {
        return colorize( _( "very poor" ), c_red );
    } else if( power >= 3 && power <= 4 ) {
        return colorize( _( "poor" ), c_light_red );
    } else if( power >= 5 && power <= 6 ) {
        return colorize( _( "average" ), c_yellow );
    } else if( power >= 7 && power <= 8 ) {
        return colorize( _( "good" ), c_yellow );
    } else if( power >= 9 && power <= 10 ) {
        return colorize( _( "very good" ), c_light_green );
    } else if( power >= 11 && power <= 12 ) {
        return colorize( _( "great" ), c_light_green );
    } else if( power >= 13 && power <= 14 ) {
        return colorize( _( "outstanding" ), c_green );
    } else if( power >= 15 ) {
        return colorize( _( "perfect" ), c_green );
    }
    if( power < 0 ) {
        debugmsg( "Converted value out of bounds." );
    }
    return "";
}
std::string texitify_bandage_power( const int power )
{
    if( power < 5 ) {
        return colorize( _( "minuscule" ), c_red );
    } else if( power < 10 ) {
        return colorize( _( "small" ), c_light_red );
    } else if( power < 15 ) {
        return colorize( _( "moderate" ), c_yellow );
    } else if( power < 20 ) {
        return colorize( _( "good" ), c_light_green );
    } else if( power < 30 ) {
        return colorize( _( "excellent" ), c_light_green );
    } else if( power < 51 ) {
        return colorize( _( "outstanding" ), c_green );
    } else {
        debugmsg( "Converted value out of bounds." );
    }
    return "";
}
nc_color colorize_bleeding_intensity( const int intensity )
{
    if( intensity == 0 ) {
        return c_unset;
    } else if( intensity < 11 ) {
        return c_light_red;
    } else if( intensity < 21 ) {
        return c_red;
    } else {
        return c_red_red;
    }
}
