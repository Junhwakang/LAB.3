#include <gtk/gtk.h>

// 위젯 포인터를 담기 위한 구조체
typedef struct {
    GtkWidget *entry1;
    GtkWidget *entry2;
    GtkWidget *result_label;
} AppWidgets;

// "더하기" 버튼 클릭 콜백
static void on_add_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    const char *text1 = gtk_entry_get_text(GTK_ENTRY(widgets->entry1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(widgets->entry2));

    double a = g_ascii_strtod(text1, NULL);
    double b = g_ascii_strtod(text2, NULL);
    double res = a + b;

    char buf[64];
    g_snprintf(buf, sizeof(buf), "결과: %.2f", res);
    gtk_label_set_text(GTK_LABEL(widgets->result_label), buf);
}

// "빼기" 버튼 클릭 콜백
static void on_sub_clicked(GtkButton *button, gpointer user_data) {
    AppWidgets *widgets = (AppWidgets *)user_data;

    const char *text1 = gtk_entry_get_text(GTK_ENTRY(widgets->entry1));
    const char *text2 = gtk_entry_get_text(GTK_ENTRY(widgets->entry2));

    double a = g_ascii_strtod(text1, NULL);
    double b = g_ascii_strtod(text2, NULL);
    double res = a - b;

    char buf[64];
    g_snprintf(buf, sizeof(buf), "결과: %.2f", res);
    gtk_label_set_text(GTK_LABEL(widgets->result_label), buf);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label1, *label2;
    GtkWidget *entry1, *entry2;
    GtkWidget *add_button, *sub_button;
    GtkWidget *result_label;

    AppWidgets widgets;

    // 1. GTK 초기화
    gtk_init(&argc, &argv);

    // 2. 윈도우 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "간단 계산기 (GTK+)");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);

    // 닫기 버튼 누르면 프로그램 종료
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 3. 그리드 레이아웃 생성
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);

    // 4. 위젯 생성
    label1 = gtk_label_new("숫자 1:");
    label2 = gtk_label_new("숫자 2:");

    entry1 = gtk_entry_new();
    entry2 = gtk_entry_new();

    add_button = gtk_button_new_with_label("더하기");
    sub_button = gtk_button_new_with_label("빼기");

    result_label = gtk_label_new("결과: ");

    // AppWidgets 구조체에 위젯 포인터 저장
    widgets.entry1 = entry1;
    widgets.entry2 = entry2;
    widgets.result_label = result_label;

    // 5. 그리드에 배치
    gtk_grid_attach(GTK_GRID(grid), label1,       0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1,       1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2,       0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2,       1, 1, 2, 1);

    gtk_grid_attach(GTK_GRID(grid), add_button,   0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sub_button,   1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), result_label, 0, 3, 3, 1);

    // 6. 버튼 클릭 시그널 연결
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_clicked), &widgets);
    g_signal_connect(sub_button, "clicked", G_CALLBACK(on_sub_clicked), &widgets);

    // 7. 모든 위젯 표시
    gtk_widget_show_all(window);

    // 8. GTK 메인 루프 진입
    gtk_main();

    return 0;
}

