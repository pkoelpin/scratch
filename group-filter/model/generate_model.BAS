Sub Main
    Dim App as femap.model
    Set App = feFemap()

    Dim tmpSet As femap.Set
    Set tmpSet = App.feSet

    Dim material As femap.Matl
    Set material = App.feMatl

    Dim count As Integer
    Dim colorID As Integer
    count = 10000

    ' Clear all Properties
    tmpSet.AddAll(FT_PROP)
        While tmpSet.Next()
        App.feProp.Delete(tmpSet.CurrentID)
    Wend

    ' Clear all Layups
    tmpSet.Clear()
    tmpSet.AddAll(FT_LAYUP)
    While tmpSet.Next()
        App.feLayup.Delete(tmpSet.CurrentID)
    Wend

    ' Clear all materials
    tmpSet.Clear()
    tmpSet.AddAll(FT_MATL)
    While tmpSet.Next()
        App.feMatl.Delete(tmpSet.CurrentID)
    Wend

    ' Clear all groups
    tmpSet.Clear()
    tmpSet.AddAll(FT_GROUP)
    While tmpSet.Next()
        App.feGroup.Delete(tmpSet.CurrentID)
    Wend

    ' Generate materials
    for i = 1 to count
        material.color = Int((124 - 0 + 1) * Rnd + 0)
        material.type = 1
        material.title = "Material " + Cstr(i)
        material.Ex = 10e6
        material.Nuxy = 0.33
        material.Put(i)
    Next i

    App.feAppMessage(FCM_NORMAL, "Complete")


End Sub
