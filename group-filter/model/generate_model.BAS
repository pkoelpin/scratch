' Random number
' Int((upperbound - lowerbound + 1) * Rnd + lowerbound)

Sub Main
    Dim App as femap.model
    Set App = feFemap()

    Dim tmpSet As femap.Set
    Set tmpSet = App.feSet

    Dim count As Long
    count = 50

    ' Clear all elements
    tmpSet.Clear()
    tmpSet.AddAll(FT_ELEM)
    App.feDelete(FT_ELEM, tmpSet.ID)

    ' Clear all nodes
    tmpSet.Clear()
    tmpSet.AddAll(FT_NODE)
    App.feDelete(FT_NODE, tmpSet.ID)

    ' Clear all Properties
    tmpSet.Clear()
    tmpSet.AddAll(FT_PROP)
    App.feDelete(FT_PROP, tmpSet.ID)

    ' Clear all Layups
    tmpSet.Clear()
    tmpSet.AddAll(FT_LAYUP)
    App.feDelete(FT_LAYUP, tmpSet.ID)

    ' Clear all materials
    tmpSet.Clear()
    tmpSet.AddAll(FT_MATL)
    App.feDelete(FT_MATL, tmpSet.ID)

    ' Clear all groups
    tmpSet.Clear()
    tmpSet.AddAll(FT_GROUP)
    App.feDelete(FT_GROUP, tmpSet.ID)

    ' Generate materials
    Dim material As femap.Matl
    Set material = App.feMatl
    for i = 1 to count*count
        material.color = Int((124 - 0 + 1) * Rnd + 0)
        material.type = 0
        material.title = "Material " + Cstr(i)
        material.Ex = 10e6
        material.Nuxy = 0.33
        material.AutoComplete()
        material.Put(i)
    Next i

    ' Generate layups
    Dim layup As femap.Layup
    Set layup = App.feLayup
    for i = 1 to count*count
        layup.clear()
        layup.title = "Layup " + Cstr(i)
        plyCount = Int((50 - 1 + 1) * Rnd + 1)
        for j = 0 to plyCount
            layup.AddPly(Int((count*count - 1 + 1) * Rnd + 1), 0.01, 90 * Rnd, 0)
        Next j
        layup.put(i)
    Next i

    ' Generate Properties
    Dim prop As femap.Prop
    Set prop = App.feProp
    for i = 1 to count*count
        prop.color = Int((124 - 0 + 1) * Rnd + 0)
        prop.title = "Property " + Cstr(i)
        prop.LayupID = ID
        prop.type = 21
        prop.put(i)
    Next i
    
    ' Generate nodes
    Dim numNode As Long
    Dim entID As Variant
    Dim xyz As Variant

    numNode = (count*count + 1) * (count*count + 1)
    Redim entID(numNode)
    Redim xyz(3 * numNode)

    Dim index As Long
    for i = 0 to count
        for j = 0 to count
            index = i * (count+1) + j
            entID(index) = index + 1  
            xyz(3*index + 0) = j*1.0
            xyz(3*index + 1) = i*1.0
            xyz(3*index + 2) = 0.0
        Next j
    Next i
    App.feNode.PutCoordArray(numNode, entID,  xyz)

    ' Generate elements
    Dim elem As femap.Elem
    Set elem = App.feElem
    elem.type = 21
    for i = 0 to count - 1 
        for j = 0 to count - 1
                index = i * (count) + j
                elem.propID = index
                elem.topology = 4
                elem.node(0) = i * (count + 1) + j + 1
                elem.node(1) = i * (count + 1) + j + 2
                elem.node(2) = i * (count + 1) + j + count + 3
                elem.node(3) = i * (count + 1) + j + count + 2
                elem.put(index + 1)
        Next j
    Next i

    ' Generate Groups
    Dim group As femap.Group
    Set group = App.feGroup
    for i = 1 to count*count
        group.Clear()
        group.title = "Group " + Cstr(i)
        group.RangeAdd(23, i, i, 1, 1)
        group.put(i)
    Next i

    App.feAppMessage(FCM_NORMAL, "Complete")


End Sub
