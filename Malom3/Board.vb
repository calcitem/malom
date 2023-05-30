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


Public Class Board
    Private s As GameState
    Private SelectedMezo As Integer = -1
    Private JelöltMezok() As Integer = {}
    Private LastJeloltMezok() As Integer = {}
    Public Advisor As PerfectPlayer

    Public Sub ClearMezoSelection()
        SelectedMezo = -1
    End Sub

    Public Sub JelolMezo(ByVal m As Integer()) 'pöttyös jelölés
        JelöltMezok = m
        If m.Length > 0 Then
            LastJeloltMezok = JelöltMezok.ToArray() 'deep copy
        End If
    End Sub
    Public Sub ShowLastJeloltMezok()
        JelolMezo(LastJeloltMezok)
        UpdateGameState() 'ez azért csak ide kell, mert normál esetben a MakeMove meghívja az UpdateUI-t
    End Sub

    Public Sub New()
    End Sub
    Public Sub UpdateGameState(ByVal _s As GameState) 'átállítja a state-et, és rendereli
        s = _s
    End Sub
    Public Sub UpdateGameState()
        UpdateGameState(s)
    End Sub

End Class