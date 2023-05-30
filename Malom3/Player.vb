' Malom, a Nine Men's Morris (and variants) player and solver program.
' Copyright(C) 2007-2016  Gabor E. Gevay, Gabor Danner
' 
' See our webpage (and the paper linked from there):
' http://compalg.inf.elte.hu/~ggevay/mills/index.php
' 
' 
' This program is free software: you can redistribute it and/or modify
' it under the terms of the GNU General Public License as published by
' the Free Software Foundation, either version 3 of the License, or
' (at your option) any later version.
' 
' This program is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU General Public License for more details.
' 
' You should have received a copy of the GNU General Public License
' along with this program.  If not, see <http://www.gnu.org/licenses/>.


Public MustInherit Class Player
    Protected G As Game
    Public Overridable Sub Enter(ByVal _g As Game) 'a paraméterben megadott játszmába való belépésrõl értesíti az objektumot
        G = _g
    End Sub
    Public Overridable Sub Quit() 'a játszmából való kilépésrõl értesíti az objektumot
        If G Is Nothing Then Return 'ez azt jelzi, hogy nem biztos, hogy ez az fv meghivodik leszarmazott osztalybol, ha mar ki vagyunk lepve
        G = Nothing
    End Sub
    Public MustOverride Sub ToMove(ByVal s As GameState) 'arról értesíti az objektumot, hogy õ következik lépni
    Public Overridable Sub FollowMove(ByVal M As Object) 'az ellenfél lépésérõl értesít

    End Sub
    Public Overridable Sub OppToMove(ByVal s As GameState) 'arról értesíti az objektumot, hogy az ellenfél következik lépni (kell pl. a gépi játékos ellenfél idejében gondolkodásához)

    End Sub
    Public Overridable Sub Over(ByVal s As GameState)

    End Sub
    Public Overridable Sub CancelThinking()

    End Sub
    Protected Function Opponent() As Player
        Return If(Object.ReferenceEquals(G.Ply(0), Me), G.Ply(1), G.Ply(0))
    End Function
End Class
