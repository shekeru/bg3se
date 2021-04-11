BEGIN_CLS(DamageSums)
P(TotalDamageDone)
P(TotalHealDone)
P(DamagePercentage)
END_CLS()


BEGIN_CLS(DamagePair)
P(Amount)
P(DamageType)
END_CLS()


BEGIN_CLS(DiceValues)
P(DiceValue)
P(AmountOfDices)
P(DiceAdditionalValue)
END_CLS()


BEGIN_CLS(DamageModifierMetadata)
P(MetadataType)
P(Value)
P(DamageType)
P(SourceType)
P_REF(Dice)
P(Source)
END_CLS()


BEGIN_CLS(StatsRollRoll)
P_REF(Roll)
P(RollType)
P(Advantage)
P(Disadvantage)
END_CLS()


BEGIN_CLS(StatsRollResult)
P(Total)
P(DiceTotal)
P(Critical)
END_CLS()


BEGIN_CLS(StatsRollBoosts)
P(ProficiencyBonus)
P(RollBonus)
P(ThrownDamage)
P(AbilityBoosts)
P(SkillBonuses)
END_CLS()


BEGIN_CLS(StatsRollBase)
P_REF(Roll)
P_REF(Result)
P_REF(Boosts)
P(AutoSkillCheckFail)
P(AutoAbilityCheckFail)
P(AutoAbilitySavingThrowFail)
P(HasCustomMetadata)
P(IsCritical)
END_CLS()


BEGIN_CLS(StatsRollType0)
INHERIT(StatsRollBase)
P(Type)
END_CLS()


BEGIN_CLS(StatsRollType1)
P(StatExpression)
P(IntParams)
// FIXME - ref array? P(RollParams)
P(DamageTypeParams)
P(IntIndex)
P(RollIndex)
P(DamageTypeIndex)
END_CLS()


BEGIN_CLS(ConditionRoll)
P(DataType)
P(RollType)
P_REF(Roll)
P(Difficulty)
END_CLS()


BEGIN_CLS(DamageResistance)
P(ResistanceType)
P(DamageType)
P(Damage)
END_CLS()


BEGIN_CLS(Hit::Subobj)
// FIXME - ref array? P(DamageRolls)
// FIXME - ref array? P(ModifierMetadata)
P_REF(ConditionRoll)
// FIXME - ref array? P(ModifierMetadata2)
// FIXME - ref array? P(Resistances)
P(AdditionalDamage)
P(TotalDamage)
P(FinalDamage)
P(TotalDamagePerType) // FIXME - ref map?
P(FinalDamagePerType) // FIXME - ref map?
END_CLS()


BEGIN_CLS(Hit)
P(TotalDamageDone)
P(TotalDamagePerceived)
P(DeathType)
P(DamageType)
P(CauseType)
P(ImpactPosition)
P(ImpactDirection)
P(ArmorAbsorption)
P(LifeSteal)
P(EffectFlags)
P(HitWith)
P(AttackRollAbility)
P(SaveAbility)
P(Inflicter)
P(InflicterOwner)
P(Throwing)
P(StoryActionId)
// FIXME - ref array? P(ConditionRolls)
P_REF(SomeSubobj)
P(SpellAttackType)
P(SpellId)
P(SpellSchool)
P(SpellLevel)
P(SpellPowerLevel)
P(TotalHealDone)
P(InstantKill)
P(FirstAttack)
// FIXME - damagelist object! P(DamageList)
END_CLS()


BEGIN_CLS(NewHit)
INHERIT(Hit)
P_REF(DamageSums)
END_CLS()