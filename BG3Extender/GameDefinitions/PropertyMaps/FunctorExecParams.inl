BEGIN_CLS(stats::BaseFunctorExecParams)
P_RO(ParamsTypeId)
P(PropertyContext)
P(StoryActionId)
P(Originator)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType1)
INHERIT(stats::BaseFunctorExecParams)
P(Caster)
P(Target)
P(Position)
P(IsFromItem)
P(SpellId)
P(Hit)
P(DamageSums)
P(SomeRadius)
P(HitWith)
P(Hit2Flags)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType2)
INHERIT(stats::BaseFunctorExecParams)
P(Caster)
P(Position)
P(ExplodeRadius)
P(IsFromItem)
P(SpellId)
P(Hit)
P(DamageSums)
P(SomeRadius)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType3)
INHERIT(stats::BaseFunctorExecParams)
P(Caster)
P(Target)
P(Position)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType4)
INHERIT(stats::BaseFunctorExecParams)
P(Caster)
P(Position)
P(SpellId)
P(Hit)
P(DamageSums)
P(Hit2_field_90)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType5)
INHERIT(stats::BaseFunctorExecParams)
P(Owner_M)
P(Target)
P(Caster)
P(Position)
P(IsFromItem)
P(SpellId)
P(Hit)
P(DamageSums)
P(SomeRadius)
P(field_268)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType6)
INHERIT(stats::BaseFunctorExecParams)
P(Target)
P(TargetX)
P(Caster)
P(Position)
P(IsFromItem)
P(SpellId)
P(Hit)
P(DamageSums)
P(SomeRadius)
P(field_268)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType7)
INHERIT(stats::BaseFunctorExecParams)
P(Caster)
P(Target)
P(SummonInInventoryFlag)
END_CLS()


BEGIN_CLS(stats::FunctorExecParamsType8)
INHERIT(stats::BaseFunctorExecParams)
P(Target)
END_CLS()
