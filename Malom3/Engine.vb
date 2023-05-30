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


Imports System.Threading

Class Engine
    Dim xorMalome, xorAP As Int64
    Dim xorStage As Int64
    Dim St2Moves(63) As Move
    Dim FlyMoves(575) As Move
    Dim CP As Integer 'a számítógép játékos sorszáma
    'Const inf As Integer = Integer.MaxValue - 1
    Const inf As Integer = Integer.MaxValue - 1000000000 '
    Const SureWin As Integer = inf - 1000
    Dim TTSize As Integer
    Dim TT() As TTElem
    Dim InvMPozSld(23, 23) As Byte 'megadja, hogy egy st2-es lépés után MPK-ban hol kell ellenőrzést csinálni
    Dim SMalomPoz(23, 23) As Boolean 'true, ha a két mező azonos malompozícióban van

    Public EndTh As Boolean 'true-ra kell állítani, ha félbe akarjuk szakítani a gondolkodást
    Public OppTime As Boolean 'jelzi, hogy a jelenlegi gondolkodás az ellenfél idejében megy-e
    Const NMVal As Byte = 2 'azoknak a mezőknek az értéke, amikből négy másik mezőre lehet lépni
    Dim BCalcLLNum As Boolean 'figyelje-e a lépéslehetőségek számát az értékelőfüggvény
    Dim NoCheck As Boolean 'az ugrálós lépéseknél a bonyolult ellenőrzés kiiktatására van, amikor nem lehet megfelelőt lépni
    Public MezoÉrtékek(23) As Integer

    Dim ThResult As ThinkResult
    Dim HasResult As Boolean

    Public UseAdv As Boolean
    Public Advisor As PerfectPlayer

    Public ThinkThread As System.Threading.Thread

    Const Ponder = False


    Public Structure Move
        Public honnan As SByte
        Public hová As SByte
        Public flm As SByte '-1 invalid, 0 fölrakás, 1 levétel, 2 mozgás, 3 ugrálás
        Public malome As Boolean
        Public Shared Operator =(ByVal a As Move, ByVal b As Move) As Boolean
            Return a.honnan = b.honnan And a.hová = b.hová
        End Operator
        Public Shared Operator <>(ByVal a As Move, ByVal b As Move) As Boolean
            Return Not (a = b)
        End Operator
    End Structure

    Private Structure TTElem
        Dim key As Int64
        Dim value As Int32
        Dim type As SByte
        Dim Bestmove As Int16
        Dim d As Byte
        Dim lc As Int16 'lépéscount
    End Structure

    Private Structure MVPair
        Dim move As Move
        Dim val As Integer
    End Structure

    Public Structure ThinkResult
        Public BestMove As Move
        Public d As Byte
        Public st As Date
        Public ev As Integer
        Public NN As Int64
    End Structure


    Dim DepthKiír, SetMainText, SetLblPerfEvalText As Action(Of String)

    Sub ignore(x As String)
    End Sub

    Dim LimitDepthForWRGM As Boolean

    Sub New(DepthKiír As Action(Of String), SetMainText As Action(Of String), SetLblPerfEvalText As Action(Of String), LimitDepthForWRGM As Boolean, Optional ByVal UseAdv As Boolean = False)
        Me.UseAdv = UseAdv
        Me.DepthKiír = If(IsNothing(DepthKiír), AddressOf ignore, DepthKiír)
        Me.SetMainText = If(IsNothing(SetMainText), AddressOf ignore, SetMainText)
        Me.SetLblPerfEvalText = If(IsNothing(SetLblPerfEvalText), AddressOf ignore, SetLblPerfEvalText)
        Me.LimitDepthForWRGM = LimitDepthForWRGM
    End Sub


    Public Sub InitEngine()
        Debug.Assert(Rules.AlphaBetaAvailable())
        Dim i, j, kov As Integer
        For i = 0 To 23
            For j = 1 To ALBoardGraph(i, 0)
                St2Moves(kov).honnan = i
                St2Moves(kov).hová = ALBoardGraph(i, j)
                St2Moves(kov).flm = 2
                kov += 1
            Next
        Next
        kov = 0
        For i = 0 To 23
            For j = 0 To 23
                If i <> j Then
                    FlyMoves(kov).honnan = i
                    FlyMoves(kov).hová = j
                    FlyMoves(kov).flm = 3
                    kov += 1
                End If
            Next
        Next
        For i = 0 To 23
            For j = 0 To 23
                If MillPos(InvMillPos(j)(0), 0) <> i And MillPos(InvMillPos(j)(0), 1) <> i And MillPos(InvMillPos(j)(0), 2) <> i Then
                    InvMPozSld(i, j) = InvMillPos(j)(0)
                End If
                If MillPos(InvMillPos(j)(1), 0) <> i And MillPos(InvMillPos(j)(1), 1) <> i And MillPos(InvMillPos(j)(1), 2) <> i Then
                    InvMPozSld(i, j) = InvMillPos(j)(1)
                End If
            Next
        Next
        For i = 0 To 15
            SMalomPoz(MillPos(i, 0), MillPos(i, 1)) = True
            SMalomPoz(MillPos(i, 0), MillPos(i, 2)) = True
            SMalomPoz(MillPos(i, 1), MillPos(i, 2)) = True
            SMalomPoz(MillPos(i, 1), MillPos(i, 0)) = True
            SMalomPoz(MillPos(i, 2), MillPos(i, 0)) = True
            SMalomPoz(MillPos(i, 2), MillPos(i, 1)) = True
        Next
        For i = 0 To 23
            MezoÉrtékek(i) = ALBoardGraph(i, 0)
        Next
        'Main.Text = "Av: " & My.Computer.Info.AvailablePhysicalMemory & " Total: " & My.Computer.Info.TotalPhysicalMemory
        'Const MaxTTSize = 70000000
        Const MaxTTSize = 5000000
        Const MinTTSize = 1000000
        If My.Computer.Info.AvailablePhysicalMemory > MaxTTSize * 24 Then
            TTSize = MaxTTSize
        Else
            If My.Computer.Info.AvailablePhysicalMemory > MinTTSize * 24 Then
                TTSize = My.Computer.Info.AvailablePhysicalMemory \ 24
            Else
                TTSize = MinTTSize
            End If
            'MsgBox("Not enough physical memory for MaxTTSize.")
        End If
        GC.Collect()
ujra:
        Try
            TT = Array.CreateInstance(GetType(TTElem), TTSize)
        Catch ex As OutOfMemoryException
            'MsgBox("Could not allocate TT with TTSize " & TTSize.ToString & ". Trying again with smaller size.")
            TTSize = TTSize * 4 / 5
            GoTo ujra
        End Try

        ResetTT()
        xorMalome = xorMezok(1, 23) * 2
        xorAP = xorMalome * 2
        xorStage = xorAP * 2
        'InitUjEval()

        If UseAdv Then Advisor = New PerfectPlayer
    End Sub

    Public Sub StopOpptime()
        EndTh = True
        If OppTime Then ThinkThread.Join()
        OppTime = False
        EndTh = False
    End Sub

    Public cancel As Boolean

    Public Sub CancelThinking()
        cancel = True
        EndTh = True
        If ThinkThread IsNot Nothing AndAlso ThinkThread.ThreadState = ThreadState.Running Then ThinkThread.Join()
        EndTh = False
        cancel = False
    End Sub



    Public Function xorMezok(ByVal ap As Integer, ByVal i As Integer) As Int64
        Return CType(1, Int64) << (i + ap * 24)
    End Function

    Private Sub ResetTT()
        Dim r As TTElem
        For i = 0 To TTSize - 1
            TT(i) = r
        Next
    End Sub

    Public s As GameState
    Dim T() As Integer 'Integer 'SByte 'tábla
    Dim AP As Integer 'eredetileg Byte volt 'Integer-ről Int64-re váltás kb 6%-os gyorsulást jelentett egyszer, de most mindegynek tunik (masik gep, mas framework verzio, de lehetett akar meresi hiba is)
    Dim KLe As Boolean 'Koronglevétel
    Dim FKC() As Integer 'Byte 'Fölrakottkorongcount
    Dim KC() As Integer 'Korongcount
    Dim Mob() As Integer 'Mobility
    Dim NV() As Integer 'a játékosok korongjai alatt lévő mezőértékek
    Dim MPK(15, 1) As Byte 'malompozíciókon lévő korongok száma a két játékosnak külön
    Dim Stage As Integer 'Byte 'szakasz
    Dim EvalMD As Integer
    Dim Hash As Integer
    Public Key As Int64 'ha ez uint64, akkor modolasnal decimal.remainder-be megy bele, ami majdnem ketszeres lassulast jelent a teljes programra nezve
    Dim MdMd As Integer 'ugráláskor díjjazza, ha cp támad
    Public NN As Int64 'vizsgált csúcsok száma
    Public Klr(KlrSize) As Integer
    Const KlrSize = 32

    Public Sub PreThinkInit()
        T = Array.CreateInstance(GetType(Integer), 24)
        For i = 0 To 23
            T(i) = s.T(i)
        Next
        AP = s.SideToMove

        'CP = AP 'ez azért nem lenne jó, mert lehet, hogy az ellenfél idejében gondolkodunk
        'If TypeOf G.Ply(0) Is ComputerPlayer Then CP = 0 Else CP = 1
        'If G.Ply(0) Is Me Then CP = 0 Else If G.Ply(1) Is Me Then CP = 1 Else Throw New InvalidOperationException("PreThinkInit nem sikerült, mert nem találom magamat a játékosok között.")
        If Not OppTime Then CP = AP Else CP = AP Xor 1

        KLe = s.KLE
        'FKC = Main.FölrakottKorongCount 'nem elég shallow copy-t csinálni
        FKC = Array.CreateInstance(GetType(Integer), 2)
        FKC(0) = s.SetStoneCount(0)
        FKC(1) = s.SetStoneCount(1)
        KC = Array.CreateInstance(GetType(Integer), 2)
        KC(0) = s.StoneCount(0)
        KC(1) = s.StoneCount(1)

        'Stage = Main.Szakasz 'ez így hibás lenne, mert a NextPlayer() nem fut le, amikor az utolsó korong fölrakása egy malomcsinálás
        If s.SetStoneCount(0) = 9 AndAlso s.SetStoneCount(1) = 9 Then Stage = 2 Else Stage = 1
        Mob = Array.CreateInstance(GetType(Integer), 2)
        NV = Array.CreateInstance(GetType(Integer), 2)
        NV(0) = KC(0) * 4
        NV(1) = KC(1) * 4
        BCalcLLNum = True 'Settings.ChkCalcLLNum.Checked
        '
        For i = 0 To 23
            If T(i) <> -1 Then
                For j = 1 To ALBoardGraph(i, 0)
                    If T(ALBoardGraph(i, j)) = -1 Then Mob(T(i)) += 1
                Next
            End If
        Next
        For i = 8 To 14 Step 2
            If T(i) > -1 Then Mob(T(i)) += NMVal
        Next
        Mob(0) += 1
        Mob(1) += 1
        'Main.Text = Mob(0) & " " & Mob(1)
        '
        Dim korr As Integer = 15
        Mob(0) += korr
        Mob(1) += korr
        For i = 0 To 15
            MPK(i, 0) = 0
            MPK(i, 1) = 0
        Next
        'MPCov(0) = 1
        'MPCov(1) = 1
        For i = 0 To 23
            If T(i) <> -1 Then
                MPK(InvMillPos(i)(0), T(i)) += 1
                MPK(InvMillPos(i)(1), T(i)) += 1
            End If
        Next
        Key = 0
        For i = 0 To 23
            If T(i) > -1 Then Key = Key Xor xorMezok(T(i), i)
        Next
        If AP = 1 Then Key = Key Xor xorAP
        If KLe Then Key = Key Xor xorMalome
        If Stage = 2 Then Key = Key Xor xorStage
        Hash = Key Mod TTSize
        MdMd = 0
        NN = 0
        EvalMD = 0
    End Sub


    Class AbortThinkingException
        Inherits Exception
    End Class

    Dim NodeCountLimit As Integer
    Private Function Lookup(ByVal d As Byte, ByRef alfa As Integer, ByRef beta As Integer, ByRef Bestmove As Short, ByRef Value As Integer) As Boolean 'Vajon miért push-olja itt az rbx-et?
        Hash = Key Mod TTSize
        If TT(Hash).key <> Key Then Return False
        If d <= TT(Hash).d Then
            If TT(Hash).type = 0 Then
                Value = TT(Hash).value
                Return True
            Else
                If TT(Hash).type = -1 Then
                    If TT(Hash).value < beta Then beta = TT(Hash).value
                    If beta <= alfa Then Value = beta : Return True '
                Else 'type = 1
                    If TT(Hash).value > alfa Then alfa = TT(Hash).value
                    If alfa >= beta Then Value = alfa : Return True '
                End If
                Bestmove = TT(Hash).Bestmove
                Return False
            End If
        Else
            Bestmove = TT(Hash).Bestmove
            Return False
        End If
    End Function
    Private Function GetMoveList() As MVPair()
        Dim MoveList() As MVPair = Array.CreateInstance(GetType(MVPair), 64)
        Dim i, NumMoves As Integer
        Dim m As Move
        For i = 0 To fLLc()
            m = GetMove(i)
            If m.flm > -1 Then
                MoveList(NumMoves).move = GetMove(i)
                NumMoves += 1
            End If
        Next
        Dim RetMoveList() As MVPair = Array.CreateInstance(GetType(MVPair), NumMoves)
        Array.Copy(MoveList, RetMoveList, NumMoves)
        Return RetMoveList
    End Function
    Private Function fLLc() As Integer 'lépéslehetőségek száma
        If KLe Then Return 23
        If Stage = 1 Then Return 23
        If Stage = 2 Then If KC(AP) > 3 Then Return 63 Else Return 575
        Return -1
    End Function
    Private Function GetMove(ByVal i As Integer) As Move
        Dim M As Move
        M.flm = -1
        M.honnan = -1
        M.hová = -1
        MdMd = 0
        If Not KLe Then
            If Stage = 1 Then 'Stage = 1
                If T(i) = -1 Then
                    M.hová = i
                    M.flm = 0
                    'M.malome = MalomeHov(M.hová)
                    If MPK(InvMillPos(M.hová)(0), AP) = 2 Then M.malome = True
                    If MPK(InvMillPos(M.hová)(1), AP) = 2 Then M.malome = True
                End If
            Else 'Stage = 2
                If KC(AP) > 3 Then 'mozgás
                    If T(St2Moves(i).honnan) = AP AndAlso T(St2Moves(i).hová) = -1 Then
                        M = St2Moves(i)
                        'M.malome = MalomeHov(M.hová)
                        If MPK(InvMPozSld(M.honnan, M.hová), AP) = 2 Then M.malome = True
                    End If
                Else 'ugrálás
                    GetFlyMove(i, M) 'ezt, és a GetKLEMove-ot azért érdemes kiemelni külön függvénybe, mert különben egy csomó register-t push-ol a függvény elején fölöslegesen, ahelyett, hogy csak ezekben a blokkokban push-olná (akkor is, ha az egyik már ki van emelve)
                End If
            End If
        Else 'Koronglevétel
            GetKLEMove(i, M)
        End If
        Return M
    End Function
    Private Sub GetFlyMove(ByVal i As Integer, ByRef M As Move)
        If T(FlyMoves(i).honnan) = AP AndAlso T(FlyMoves(i).hová) = -1 AndAlso
                        (NoCheck OrElse (KC(1 - AP) > 3 OrElse If(SMalomPoz(FlyMoves(i).honnan, FlyMoves(i).hová),
                        (MPK(InvMPozSld(FlyMoves(i).honnan, FlyMoves(i).hová), AP) >= 1 AndAlso MPK(InvMPozSld(FlyMoves(i).honnan, FlyMoves(i).hová), 1 - AP) = 0) OrElse MPK(InvMPozSld(FlyMoves(i).honnan, FlyMoves(i).hová), 1 - AP) = 2,
                        (MPK(InvMillPos(FlyMoves(i).hová)(0), AP) >= 1 AndAlso MPK(InvMillPos(FlyMoves(i).hová)(0), 1 - AP) = 0) OrElse MPK(InvMillPos(FlyMoves(i).hová)(0), 1 - AP) = 2 OrElse
                        (MPK(InvMillPos(FlyMoves(i).hová)(1), AP) >= 1 AndAlso MPK(InvMillPos(FlyMoves(i).hová)(1), 1 - AP) = 0) OrElse MPK(InvMillPos(FlyMoves(i).hová)(1), 1 - AP) = 2))) Then
            M = FlyMoves(i)
            'M.malome = MalomeHov(M.hová) 'így hibás lenne
            'If MPK(InvMalomPoz(M.hová, 0), AP) = 2 Then M.malome = True
            'If MPK(InvMalomPoz(M.hová, 1), AP) = 2 Then M.malome = True
            T(M.hová) = AP
            T(M.honnan) = -1
            M.malome = Malome(M.hová)
            T(M.honnan) = AP
            T(M.hová) = -1
            Dim mhov As Byte = M.hová
            If AP = CP Then If MPK(InvMillPos(mhov)(0), 1 - AP) = 2 OrElse MPK(InvMillPos(mhov)(1), 1 - AP) = 2 Then MdMd = -10000 Else MdMd = 10000
        End If
    End Sub
    Private Sub GetKLEMove(ByVal i As Integer, ByRef m As Move)
        If T(i) = 1 - AP AndAlso ((MPK(InvMillPos(i)(0), 1 - AP) <> 3 AndAlso MPK(InvMillPos(i)(1), 1 - AP) <> 3) OrElse KC(1 - AP) = 3) Then
            m.honnan = i
            m.flm = 1
        End If
    End Sub
    Private Function Malome(ByVal mezo As Byte) As Boolean
        Malome = False
        'lehetne cache-elni a T(mezo)-t, meg az InvMillPos(mezo)(i)-t
        For i As Integer = 0 To 1
            If T(MillPos(InvMillPos(mezo)(i), 0)) = T(mezo) AndAlso T(MillPos(InvMillPos(mezo)(i), 1)) = T(mezo) AndAlso T(MillPos(InvMillPos(mezo)(i), 2)) = T(mezo) Then
                Malome = True 'ezt at kene irni returnre
            End If
        Next
    End Function

    Structure UndoData
        Dim Mob0, Mob1 As Integer
        Dim malome As Boolean
        Dim eval As Integer
        Dim key As Int64
    End Structure
    Private Sub DoMove(ByVal M As Move, ByRef UD As UndoData)
        UD.malome = False
        UD.eval = 0
        UD.Mob0 = Mob(0)
        UD.Mob1 = Mob(1)
        If Not M.malome Then UD.eval = NTrEval() + MdMd
        EvalMD += UD.eval
        UD.key = Key
        Select Case M.flm '-1 invalid, 0 fölrakás, 1 levétel, 2 mozgás, 3 ugrálás
            Case 0
                T(M.hová) = AP
                KC(AP) += 1
                FKC(AP) += 1
                If FKC(0) = 9 And FKC(1) = 9 Then
                    Stage = 2
                    Key = Key Xor xorStage
                End If
                CalcMob(M.honnan, M.hová)
                'If Settings.CalcNodeValues Then NV(AP) += MezőÉrtékek(M.hová)
                MPK(InvMillPos(M.hová)(0), AP) += 1
                MPK(InvMillPos(M.hová)(1), AP) += 1
                Key = Key Xor xorMezok(AP, M.hová)
                If M.malome Then Key = Key Xor xorMalome
            Case 1
                T(M.honnan) = -1
                KC(1 - AP) -= 1
                CalcMobKLe(M.honnan)
                MPK(InvMillPos(M.honnan)(0), 1 - AP) -= 1
                MPK(InvMillPos(M.honnan)(1), 1 - AP) -= 1
                Key = Key Xor xorMezok(1 - AP, M.honnan)
                Key = Key Xor xorMalome
            Case 2, 3
                T(M.honnan) = -1
                CalcMob(M.honnan, M.hová)
                T(M.hová) = AP
                MPK(InvMillPos(M.hová)(0), AP) += 1
                MPK(InvMillPos(M.hová)(1), AP) += 1
                MPK(InvMillPos(M.honnan)(0), AP) -= 1
                MPK(InvMillPos(M.honnan)(1), AP) -= 1
                Key = Key Xor xorMezok(AP, M.honnan)
                Key = Key Xor xorMezok(AP, M.hová)
                If M.malome Then Key = Key Xor xorMalome
        End Select
        If M.flm <> 1 Then
            If M.malome Then
                KLe = True
                UD.malome = True
            Else
                AP = 1 - AP
                Key = Key Xor xorAP
            End If
        Else 'levétel
            AP = 1 - AP
            Key = Key Xor xorAP
            KLe = False
        End If
    End Sub
    Structure UndoDataETC
        Dim key As Int64
    End Structure
    Private Function DoMoveETC(ByVal M As Move) As UndoDataETC
        Dim UD As UndoDataETC
        UD.key = Key
        Select Case M.flm '-1 invalid, 0 fölrakás, 1 levétel, 2 mozgás, 3 ugrálás
            Case 0
                FKC(AP) += 1
                If FKC(0) = 9 And FKC(1) = 9 Then
                    Key = Key Xor xorStage
                End If
                FKC(AP) -= 1 'talán gyorsabb itt visszavonni, és úgyse használjuk az UndoMoveETC előtt
                Key = Key Xor xorMezok(AP, M.hová)
                If M.malome Then Key = Key Xor xorMalome
            Case 1
                Key = Key Xor xorMezok(1 - AP, M.honnan)
                Key = Key Xor xorMalome
            Case 2, 3
                Key = Key Xor xorMezok(AP, M.honnan)
                Key = Key Xor xorMezok(AP, M.hová)
                If M.malome Then Key = Key Xor xorMalome
        End Select
        If M.flm <> 1 Then
            If M.malome Then
            Else
                Key = Key Xor xorAP
            End If
        Else 'levétel
            Key = Key Xor xorAP
        End If
        Return UD
    End Function
    Private Sub UndoMove(ByVal M As Move, ByVal UD As UndoData)
        Mob(0) = UD.Mob0
        Mob(1) = UD.Mob1
        EvalMD -= UD.eval
        Key = UD.key
        If M.flm <> 1 Then
            If UD.malome Then
                KLe = False
            Else
                AP = 1 - AP
            End If
        Else 'levétel
            AP = 1 - AP
            KLe = True
        End If
        Select Case M.flm '-1 invalid, 0 fölrakás, 1 levétel, 2 mozgás, 3 ugrálás
            Case 0
                T(M.hová) = -1
                KC(AP) -= 1
                If FKC(0) = 9 And FKC(1) = 9 Then Stage = 1
                FKC(AP) -= 1
                MPK(InvMillPos(M.hová)(0), AP) -= 1
                MPK(InvMillPos(M.hová)(1), AP) -= 1
                'If Settings.CalcNodeValues Then NV(AP) -= MezőÉrtékek(M.hová)
            Case 1
                T(M.honnan) = 1 - AP
                KC(1 - AP) += 1
                MPK(InvMillPos(M.honnan)(0), 1 - AP) += 1
                MPK(InvMillPos(M.honnan)(1), 1 - AP) += 1
                'If Settings.CalcNodeValues Then NV(1 - AP) += MezőÉrtékek(M.honnan)
            Case 2, 3
                T(M.honnan) = AP
                T(M.hová) = -1
                MPK(InvMillPos(M.hová)(0), AP) -= 1
                MPK(InvMillPos(M.hová)(1), AP) -= 1
                MPK(InvMillPos(M.honnan)(0), AP) += 1
                MPK(InvMillPos(M.honnan)(1), AP) += 1
                'If Settings.CalcNodeValues Then NV(AP) -= MezőÉrtékek(M.hová) - MezőÉrtékek(M.honnan)
        End Select
    End Sub

    Const NMMask As Integer = 21760 '101 0101 0000 0000

    Private Sub CalcMob(ByVal honnan As SByte, ByVal hová As SByte)
        Dim i As Byte
        If honnan > -1 Then
            For i = 1 To ALBoardGraph(honnan, 0)
                Select Case T(ALBoardGraph(honnan, i))
                    Case -1 : Mob(AP) -= 1
                    Case AP : Mob(AP) += 1
                        'Case 1 - AP : Mob(1 - AP) += 1
                    Case Else : Mob(1 - AP) += 1
                End Select
            Next
            If ((1 << honnan) And NMMask) <> 0 Then Mob(AP) -= NMVal
        End If
        If hová > -1 Then
            For i = 1 To ALBoardGraph(hová, 0)
                Select Case T(ALBoardGraph(hová, i))
                    Case -1 : Mob(AP) += 1
                    Case AP : Mob(AP) -= 1
                        'Case 1 - AP : Mob(1 - AP) -= 1
                    Case Else : Mob(1 - AP) -= 1
                End Select
            Next
            If ((1 << hová) And NMMask) <> 0 Then Mob(AP) += NMVal
        End If
    End Sub
    Private Sub CalcMobKLe(ByVal honnan As SByte)
        Dim i As Byte
        If honnan > -1 Then
            For i = 1 To ALBoardGraph(honnan, 0)
                Select Case T(ALBoardGraph(honnan, i))
                    Case -1 : Mob(1 - AP) -= 1
                    Case AP : Mob(AP) += 1
                    Case 1 - AP : Mob(1 - AP) += 1
                End Select
            Next
            For i = 8 To 14 Step 2
                If honnan = i Then Mob(1 - AP) -= NMVal
            Next
        End If
    End Sub
    Private Function Eval() As Integer
        Dim r As Integer

        If KC(0) = 0 Or KC(1) = 0 Then
            KC(0) += 1
            KC(1) += 1
            r = Eval()
            KC(0) -= 1
            KC(1) -= 1
            Return r
        End If

        If BCalcLLNum Then
            If Stage = 1 Then
                r = 500000 * KC(CP) \ KC(1 - CP) + 500000 * Mob(CP) \ Mob(1 - CP) 'ez az alap
            Else
                If KC(1 - CP) > 3 And KC(CP) > 3 Then
                    r = 666666 * KC(CP) \ KC(1 - CP) + 333333 * Mob(CP) \ Mob(1 - CP) 'ennyi itt az alap
                Else
                    r = 1000000 * KC(CP) \ KC(1 - CP)
                End If
            End If
        Else
            'If Settings.CalcNodeValues Then
            'r = 1000000 * NV(CP) \ NV(1 - CP)
            'Else
            r = 1000000 * KC(CP) \ KC(1 - CP)
            'End If
        End If
        r += EvalMD
        If AP = CP Then Return r Else Return -r
    End Function
    Private Function NTrEval() As Integer
        Dim r As Integer
        If Stage = 1 Then
            'r = 1000000 * KC(CP) \ KC(1 - CP) + 1000000 * Mob(CP) \ Mob(1 - CP)
            If KC(1 - CP) > 0 Then r = 100 * KC(CP) \ KC(1 - CP) Else r = 0
        Else
            r = 100 * KC(CP) \ KC(1 - CP)
        End If
        Return r
    End Function
End Class